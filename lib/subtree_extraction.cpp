#include "trees_impl.hpp"
#include "utils.hpp"
#include <terraces/errors.hpp>
#include <terraces/subtree_extraction.hpp>

using std::vector;
using std::stack;
using std::pair;
using std::make_pair;

namespace terraces {

std::vector<tree> subtrees(const tree& t, const bitmatrix& occ) {
	auto num_nodes = num_nodes_from_leaves(occ.rows());
	auto num_sites = occ.cols();
	utils::ensure<bad_input_error>(t.size() == num_nodes,
	                               "bitmatrix and tree have incompatible sizes");
	check_rooted_tree(t);
	auto node_occ = bitmatrix{t.size(), occ.cols()};
	std::vector<index> num_leaves_per_site(occ.cols(), 0);

	// compute occurrences on inner nodes: bitwise or of the children
	foreach_postorder(t, [&](index i) {
		auto node = t[i];
		if (is_leaf(node)) {
			// copy data from taxon occurrence matrix
			assert(node.taxon() != none && "leaf without taxon ID");
			for (index site = 0; site < num_sites; ++site) {
				auto has_leaf = occ.get(node.taxon(), site);
				node_occ.set(i, site, has_leaf);
				num_leaves_per_site[site] += has_leaf;
			}
		} else {
			node_occ.row_or(node.lchild(), node.rchild(), i);
		}
	});

	// collect leaves and inner nodes: bitwise and of the children
	vector<tree> out_trees(num_sites);
	vector<stack<index>> tree_boundaries{num_sites, stack<index>{}};
	for (index site = 0; site < num_sites; ++site) {
		out_trees[site].reserve(num_nodes_from_leaves(num_leaves_per_site[site]));
		out_trees[site].emplace_back();
	}

	foreach_preorder(t, [&](index i) {
		auto node = t[i];
		for (index site = 0; site < num_sites; ++site) {
			auto& out_tree = out_trees[site];
			auto& boundary = tree_boundaries[site];

			bool leaf_occ = is_leaf(node) && node_occ.get(i, site);
			bool inner_occ = !is_leaf(node) && node_occ.get(node.lchild(), site) &&
			                 node_occ.get(node.rchild(), site);
			if (leaf_occ || (inner_occ && !is_root(node))) {
				// fires if the tree is trivial (i.e. only one edge!)
				// this can only happen with sites for which only one species has
				// data.
				assert(!boundary.empty());
				auto parent = boundary.top();
				out_tree.emplace_back(parent, none, none, node.taxon());
				if (out_tree[parent].lchild() == none) {
					out_tree[parent].lchild() = out_tree.size() - 1;
				} else {
					assert(out_tree[parent].rchild() == none);
					out_tree[parent].rchild() = out_tree.size() - 1;
					boundary.pop();
				}
			}
			if (inner_occ) {
				boundary.push(out_tree.size() - 1);
			}
		}
	});

	return out_trees;
}

} // namespace terraces
