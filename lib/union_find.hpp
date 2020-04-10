#ifndef TERRACES_UNION_FIND_HPP
#define TERRACES_UNION_FIND_HPP

#include <algorithm>

#include <terraces/trees.hpp>

#include "stack_allocator.hpp"

namespace terraces {

class union_find {
public:
	using value_type = index_t;

private:
	std::vector<index_t, utils::stack_allocator<index_t>> m_parent;
#ifndef NDEBUG
	bool m_compressed;
#endif // NDEBUG

public:
	union_find(index_t, utils::stack_allocator<index_t> a);
	index_t find(index_t);
	index_t simple_find(index_t x) const {
		assert(m_compressed);
		return is_representative(x) ? x : m_parent[x];
	}
	index_t size() const { return m_parent.size(); }
	void compress();
	void merge(index_t, index_t);
	bool is_representative(index_t x) const { return m_parent[x] >= m_parent.size(); }

	static union_find make_bipartition(const std::vector<bool>& split,
	                                   utils::stack_allocator<index_t> alloc);
};

} // namespace terraces

#endif // TERRACES_UNION_FIND_HPP
