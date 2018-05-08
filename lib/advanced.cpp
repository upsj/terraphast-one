#include <terraces/advanced.hpp>

#include <terraces/clamped_uint.hpp>
#include <terraces/errors.hpp>
#include <terraces/rooting.hpp>
#include <terraces/subtree_extraction.hpp>

#include "supertree_enumerator.hpp"
#include "supertree_variants.hpp"
#include "supertree_variants_multitree.hpp"

namespace terraces {

supertree_data create_supertree_data(const tree& tree, const bitmatrix& data) {
	auto root = find_comprehensive_taxon(data);
	utils::ensure<no_usable_root_error>(root != none, "No comprehensive taxon found");
	auto rerooted_tree = tree;
	reroot_at_taxon_inplace(rerooted_tree, root);
	auto trees = subtrees(rerooted_tree, data);
	auto constraints = compute_constraints(trees);
	deduplicate_constraints(constraints);

	auto num_leaves = data.rows();
	return {constraints, num_leaves, root};
}

index find_comprehensive_taxon(const bitmatrix& data) {
	for (index i = 0; i < data.rows(); ++i) {
		bool comp = true;
		for (index j = 0; j < data.cols(); ++j) {
			comp &= data.get(i, j);
		}
		if (comp) {
			return i;
		}
	}
	return none;
}

bitmatrix maximum_comprehensive_columnset(const bitmatrix& data) {
	std::vector<index> row_counts(data.rows(), 0u);
	for (index i = 0; i < data.rows(); ++i) {
		for (index j = 0; j < data.cols(); ++j) {
			row_counts[i] += data.get(i, j) ? 1u : 0u;
		}
	}
	auto it = std::max_element(row_counts.begin(), row_counts.end());
	index comp_row = static_cast<index>(std::distance(row_counts.begin(), it));
	utils::ensure<bad_input_error>(*it > 0, "bitmatrix contains only zeros");
	std::vector<index> columns;
	for (index j = 0; j < data.cols(); ++j) {
		if (data.get(comp_row, j)) {
			columns.push_back(j);
		}
	}
	return data.get_cols(columns);
}

bool check_terrace(const supertree_data& data) {
	tree_enumerator<variants::check_callback> enumerator{{}};
	try {
		return enumerator.run(data.num_leaves, data.constraints, data.root) > 1;
	} catch (terraces::tree_count_overflow_error&) {
		return true;
	}
}

uint64_t count_terrace(const supertree_data& data) {
	tree_enumerator<variants::clamped_count_callback> enumerator{{}};
	try {
		return enumerator.run(data.num_leaves, data.constraints, data.root).value();
	} catch (terraces::tree_count_overflow_error&) {
		return std::numeric_limits<uint64_t>::max();
	}
}

big_integer count_terrace_bigint(const supertree_data& data) {
	tree_enumerator<variants::count_callback<big_integer>> enumerator{{}};
	return enumerator.run(data.num_leaves, data.constraints, data.root);
}

big_integer print_terrace(const supertree_data& data, const name_map& names, std::ostream& output) {
	tree_enumerator<variants::multitree_callback> enumerator{{}};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	output << as_newick(result, names);

	return result->num_trees;
}

} // namespace terraces
