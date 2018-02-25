#include <terraces/advanced.hpp>

#include <terraces/clamped_uint.hpp>
#include <terraces/errors.hpp>
#include <terraces/subtree_extraction.hpp>

#include "supertree_enumerator.hpp"
#include "supertree_variants.hpp"
#include "supertree_variants_multitree.hpp"

namespace terraces {

supertree_data prepare_constraints(const tree& tree, const bitmatrix& data, index root) {
	auto trees = subtrees(tree, data);
	auto constraints = compute_constraints(trees);
	deduplicate_constraints(constraints);

	auto num_leaves = data.rows();
	return {constraints, num_leaves, root};
}

std::pair<bitmatrix, index> maximum_comprehensive_columnset(const bitmatrix& data) {
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
	return {data.get_cols(columns), comp_row};
}

bool check_terrace(const supertree_data& data) {
	tree_enumerator<variants::check_callback> enumerator{
	        {}, data.num_leaves, data.constraints.size()};
	try {
		return enumerator.run(data.num_leaves, data.constraints, data.root) > 1;
	} catch (terraces::tree_count_overflow_error&) {
		return true;
	}
}

uint64_t count_terrace(const supertree_data& data) {
	tree_enumerator<variants::clamped_count_callback> enumerator{
	        {}, data.num_leaves, data.constraints.size()};
	try {
		return enumerator.run(data.num_leaves, data.constraints, data.root).value();
	} catch (terraces::tree_count_overflow_error&) {
		return std::numeric_limits<uint64_t>::max();
	}
}

big_integer count_terrace_bigint(const supertree_data& data) {
	tree_enumerator<variants::count_callback<big_integer>> enumerator{
	        {}, data.num_leaves, data.constraints.size()};
	return enumerator.run(data.num_leaves, data.constraints, data.root);
}

big_integer print_terrace(const supertree_data& data, const name_map& names, std::ostream& output) {
	tree_enumerator<variants::multitree_callback> enumerator{
	        {}, data.num_leaves, data.constraints.size()};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	output << as_newick(result, names);

	return result->num_trees;
}

} // namespace terraces
