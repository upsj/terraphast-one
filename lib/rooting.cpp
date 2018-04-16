#include "trees_impl.hpp"
#include "utils.hpp"
#include <sstream>
#include <stack>
#include <terraces/rooting.hpp>

namespace terraces {

void reroot_inplace(tree& t, index comp_taxon) {
	// identify node corresponding to comp_taxon
	index root_leaf = none;
	for (index i = 0; i < t.size(); ++i) {
		if (t[i].taxon() == comp_taxon) {
			assert(root_leaf == none);
			root_leaf = i;
		}
	}
	assert(root_leaf != none && "The tree doesn't contain the given taxon");
	terraces::check_rooted_tree(t);

	// first: swap at inner nodes s.t. root_leaf is the rightmost leaf
	index cur = root_leaf;
	index p = t[cur].parent();
	while (cur != 0) {
		// if cur lies in the left subtree, swap it to the right
		if (t[p].lchild() == cur) {
			std::swap(t[p].lchild(), t[p].rchild());
		}
		cur = std::exchange(p, t[p].parent());
	}
	// second: move the root down right until we meet root_leaf
	auto& li = t[0].lchild();
	auto& ri = t[0].rchild();
	while (ri != root_leaf) {
		auto& ln = t[li];
		auto& rn = t[ri];
		auto& rli = rn.lchild();
		auto& rri = rn.rchild();
		auto& rrn = t[rri];
		std::swap(rrn.parent(), ln.parent());
		std::tie(li, ri, rli, rri) = std::make_tuple(ri, rri, li, rli);
	}
}

} // namespace terraces
