#include <numeric>
#include <unordered_map>

#include <algorithm>

#include "ranked_bitvector.hpp"
#include "supertree_helpers.hpp"
#include "trees_impl.hpp"
#include "validation.hpp"

namespace terraces {

std::vector<simple_bitvector> tree_bipartitions(const tree& t) {
	std::vector<simple_bitvector> bips(t.size(), {0, {}});
	std::vector<simple_bitvector> subtrees(t.size(), {(t.size() + 1) / 2, {}});
	foreach_postorder(t, [&](index i) {
		auto n = t[i];
		if (is_leaf(n)) {
			subtrees[i].set(n.taxon());
		} else {
			subtrees[i].set_bitwise_or(subtrees[n.lchild()], subtrees[n.rchild()]);
		}
	});
	foreach_preorder(t, [&](index i) {
		auto n = t[i];
		bool at_root = is_root(n);
		bool at_rhs_of_root =
		        !at_root && (is_root(t[n.parent()]) && t[n.parent()].rchild() == i);
		if (!(at_root || at_rhs_of_root)) {
			if (subtrees[i].get(0)) {
				subtrees[i].invert();
			}
			bips[i] = std::move(subtrees[i]);
		}
	});
	bips[t[0].rchild()] = std::move(subtrees[t[0].rchild()]);
	bips[t[0].rchild()].blank();
	bips[0] = std::move(subtrees[0]);
	bips[0].blank();
	std::sort(bips.begin(), bips.end());
	return bips;
}

bool is_isomorphic(const tree& fst, const tree& snd) {
	assert(fst.size() == snd.size());

	auto fst_bip = tree_bipartitions(fst);
	auto snd_bip = tree_bipartitions(snd);

	return fst_bip == snd_bip;
}

} // namespace terraces
