#include "../lib/trees_impl.hpp"
#include <algorithm>
#include <fstream>
#include <terraces/advanced.hpp>
#include <terraces/bitmatrix.hpp>
#include <terraces/errors.hpp>
#include <terraces/parser.hpp>
#include <terraces/terraces_old.h>

missingData* initializeMissingData(size_t numberOfSpecies, size_t numberOfPartitions,
                                   const char** speciesNames) {
	missingData* data = new missingData;
	data->numberOfSpecies = numberOfSpecies;
	data->numberOfPartitions = numberOfPartitions;
	data->allocatedNameArray = false; // TODO What is this entry?
	data->speciesNames = speciesNames;
	data->missingDataMatrix = new unsigned char[numberOfSpecies * numberOfPartitions];
}

void freeMissingData(missingData* m) {
	delete[] m->missingDataMatrix;
	delete m;
}

void setDataMatrix(missingData* m, size_t speciesNumber, size_t partitionNumber,
                   unsigned char value) {
	m->missingDataMatrix[speciesNumber * m->numberOfPartitions + partitionNumber] = value;
}

unsigned char getDataMatrix(const missingData* m, size_t speciesNumber, size_t partitionNumber) {
	return m->missingDataMatrix[speciesNumber * m->numberOfPartitions + partitionNumber];
}

void copyDataMatrix(const unsigned char* matrix, missingData* m) {
	std::copy_n(matrix, m->numberOfSpecies * m->numberOfPartitions, m->missingDataMatrix);
}

CHECK_RESULT int terraceAnalysis(missingData* m, const char* newickTreeString, const int ta_outspec,
                                 const char* allTreesOnTerrace, mpz_t terraceSize) {
	// check ta_outspec
	bool detect = ta_outspec & TA_DETECT;
	bool count = ta_outspec & TA_COUNT;
	bool enumerate = ta_outspec & TA_ENUMERATE;
	bool compress = ta_outspec & TA_ENUMERATE_COMPRESS;
	bool force_comprehensive = ta_outspec & TA_UPPER_BOUND;
	bool invalid1 = detect && (count || enumerate); // cannot detect and count at the same time
	bool invalid2 = compress && !enumerate;         // cannot compress if we don't enumerate
	if (invalid1 || invalid2) {
		return TERRACE_FLAG_CONFLICT_ERROR;
	}

	// check input sizes
	if (m->numberOfPartitions < 2) {
		return TERRACE_NUM_PARTITIONS_ERROR;
	}
	if (m->numberOfSpecies < 4) {
		return TERRACE_NUM_SPECIES_ERROR;
	}

	// copy missing data matrix
	terraces::bitmatrix matrix{m->numberOfSpecies, m->numberOfPartitions};
	for (size_t row = 0; row < m->numberOfSpecies; ++row) {
		size_t count = 0;
		for (size_t col = 0; col < m->numberOfPartitions; ++col) {
			auto val = m->missingDataMatrix[row * m->numberOfPartitions + col];
			if (val != 0 && val != 1) {
				return TERRACE_MATRIX_ERROR;
			}
			matrix.set(row, col, val);
			count += val;
		}
		if (count == 0) {
			return TERRACE_SPECIES_WITHOUT_PARTITION_ERROR;
		}
	}

	// copy names
	terraces::name_map names;
	terraces::index_map name_index;
	for (size_t spec_i = 0; spec_i < m->numberOfSpecies; ++spec_i) {
		names.emplace_back(m->speciesNames[spec_i]);
		if (!name_index.insert({names.back(), spec_i}).second) {
			return TERRACE_SPECIES_ERROR;
		}
	}

	// parse newick tree
	terraces::tree tree;
	try {
		tree = terraces::parse_nwk(newickTreeString, name_index);
	} catch (const terraces::bad_input_error& err) {
		return TERRACE_NEWICK_ERROR;
	}
	if (terraces::num_leaves_from_nodes(tree.size()) != m->numberOfSpecies) {
		return TERRACE_SPECIES_ERROR;
	}

	// prepare data
	if (ta_outspec & TA_UPPER_BOUND) {
		matrix = terraces::maximum_comprehensive_columnset(matrix);
	}
	auto data = terraces::create_supertree_data(tree, matrix);

	// enumerate terrace
	if (detect) {
		auto lb = terraces::fast_count_terrace(data);
		mpz_set_ui(terraceSize, lb);
	} else if (count) {
		auto size = terraces::count_terrace_bigint(data);
		mpz_set(terraceSize, size.value().get_mpz_t());
	} else {
		auto ofs = std::ofstream{allTreesOnTerrace};
		if (not ofs.is_open()) {
			return TERRACE_OUTPUT_FILE_ERROR;
		}
		mpz_class count;
		try {
			if (compress) {
				count = terraces::print_terrace_compressed(data, names, ofs)
				                .value();
			} else {
				count = terraces::print_terrace(data, names, ofs).value();
			}
		} catch (std::ifstream::failure&) {
			return TERRACE_OUTPUT_FILE_ERROR;
		}
	}
	return TERRACE_SUCCESS;
}
