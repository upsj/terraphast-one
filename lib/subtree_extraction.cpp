#include "trees_impl.hpp"
#include "utils.hpp"
#include <terraces/subtree_extraction.hpp>

using std::vector;
using std::stack;
using std::pair;
using std::make_pair;

namespace terraces {

std::vector<tree> subtrees(const tree& t, const bitmatrix& occ) {
	auto num_nodes = num_nodes_from_leaves(occ.rows());
	auto num_sites = occ.cols();
	assert(t.size() == num_nodes && "bitmatrix and tree have incompatible sizes");
	check_rooted_tree(t);

	vector<tree> out_trees(num_sites, tree(num_nodes, node{}));
	vector<stack<index>> tree_boundaries{num_sites, stack<index>{}};

	auto node_occ = bitmatrix{t.size(), occ.cols()};

	// compute occurrences on inner nodes: bitwise or of the children
	foreach_postorder(t, [&](index i) {
		auto node = t[i];
		if (is_leaf(node)) {
			// copy data from taxon occurrence matrix
			assert(node.taxon() != none && "leaf without taxon ID");
			for (index site = 0; site < num_sites; ++site) {
				node_occ.set(i, site, occ.get(node.taxon(), site));
			}
		} else {
			node_occ.row_or(node.lchild(), node.rchild(), i);
		}
	});

	// collect leaves and inner nodes: bitwise and of the children
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
				out_tree[i].parent() = parent;
				out_tree[i].taxon() = node.taxon();
				if (out_tree[parent].lchild() == none) {
					out_tree[parent].lchild() = i;
				} else {
					assert(out_tree[parent].rchild() == none);
					out_tree[parent].rchild() = i;
					boundary.pop();
				}
			}
			if (inner_occ) {
				boundary.push(i);
			}
		}
	});

	return out_trees;
}

} // namespace terraces
