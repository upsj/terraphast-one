#ifndef TERRACES_UNION_FIND_HPP
#define TERRACES_UNION_FIND_HPP

#include <algorithm>

#include "trees.hpp"

namespace terraces {

class union_find {
public:
	using value_type = index;

private:
	// mutable because of path compression
	mutable std::vector<index> m_parent;

public:
	union_find(index);
	index find(index) const;
	index size() const;
	void merge(index, index);
	bool is_representative(index) const;
};

} // namespace terraces

#endif
