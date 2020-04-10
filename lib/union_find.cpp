#include "union_find.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>

namespace terraces {

union_find::union_find(index_t n, utils::stack_allocator<index_t> a) : m_parent(n, n, a) {
#ifndef NDEBUG
	m_compressed = true;
#endif // NDEBUG
}

index_t union_find::find(index_t x) {
	assert(x < m_parent.size());
	index_t root = x;
	while (!is_representative(root)) {
		root = m_parent[root];
	}
	while (x != root) {
		x = utils::exchange(m_parent[x], root);
	}
	assert(is_representative(root) && root < m_parent.size());
	return root;
}

void union_find::compress() {
	for (index_t i = 0; i < m_parent.size(); ++i) {
		find(i);
	}
#ifndef NDEBUG
	m_compressed = true;
#endif // NDEBUG
}

void union_find::merge(index_t x, index_t y) {
#ifndef NDEBUG
	m_compressed = false;
#endif // NDEBUG
	index_t i = find(x);
	index_t j = find(y);
	if (i == j) {
		return;
	}
	if (m_parent[i] < m_parent[j]) {
		// link the smaller group to the larger one
		m_parent[i] = j;
	} else if (m_parent[i] > m_parent[j]) {
		// link the smaller group to the larger one
		m_parent[j] = i;
	} else {
		// equal rank: link arbitrarily and increase rank
		m_parent[j] = i;
		++m_parent[i];
	}
}

union_find union_find::make_bipartition(const std::vector<bool>& split,
                                        utils::stack_allocator<index_t> alloc) {
	union_find result(split.size(), alloc);
	std::array<index_t, 2> fst{{none, none}};
	for (index_t i = 0; i < split.size(); ++i) {
		auto& repr = fst[split[i]];
		repr = repr == none ? i : repr;
		result.merge(repr, i);
	}
	result.compress();
	return result;
}

} // namespace terraces
