#include "trees_impl.hpp"
#include "utils.hpp"
#include <algorithm>
#include <ostream>

namespace terraces {

void check_rooted_tree(const tree& t) {
	// check edge cases
	utils::ensure<std::invalid_argument>(t.size() != 0, "tree is empty");
	utils::ensure<std::invalid_argument>((t.size() != 1 || (is_leaf(t[0]) && is_root(t[0]))),
	                                     "invalid trivial tree");

	// check if parent/child assignment is symmetric
	for (index i = 0; i < t.size(); ++i) {
		auto n = t[i];
		if (is_leaf(n)) {
			// leaf l: child of parent is correct
			auto p = n.parent();
			utils::ensure<std::invalid_argument>(p < t.size(), "parent overflow");
			utils::ensure<std::invalid_argument>(
			        (t[p].lchild() == i || t[p].rchild() == i),
			        "leaf's parent doesn't point to leaf");
		} else {
			// inner n: parent of lc and rc is correct
			auto lc = n.lchild();
			auto rc = n.rchild();
			utils::ensure<std::invalid_argument>(lc < t.size(), "lchild overflow");
			utils::ensure<std::invalid_argument>(rc < t.size(), "rchild overflow");
			utils::ensure<std::invalid_argument>(t[lc].parent() == i &&
			                                             t[rc].parent() == i,
			                                     "nodes children don't point to node");
			utils::ensure<std::invalid_argument>(lc != rc, "lchild == rchild");
		}
	}
	utils::ensure<std::invalid_argument>(is_root(t[0]), "first node is not the root");
}

std::ostream& operator<<(std::ostream& s, newick_t tree_pair) {
	const auto& t = *tree_pair.t;
	const auto& names = *tree_pair.names;
	auto pre_cb = [&](index) { s << '('; };
	auto post_cb = [&](index) { s << ')'; };
	auto leaf_cb = [&](index i) {
		if (t[i].taxon() != none)
			s << names[t[i].taxon()];
	};
	auto sibling_cb = [&](index) { s << ','; };
	index root = 0;
	tree_traversal(t, pre_cb, post_cb, sibling_cb, leaf_cb, root);
	s << ';';
	return s;
}

std::vector<index> preorder(const tree& t) {
	std::vector<index> result;
	foreach_preorder(t, [&](index i) { result.push_back(i); });
	return result;
}

std::vector<index> postorder(const tree& t) {
	std::vector<index> result;
	foreach_postorder(t, [&](index i) { result.push_back(i); });
	return result;
}

} // namespace terraces
