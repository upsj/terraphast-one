#ifndef UNCONSTRAINED_ENUMERATOR_CPP
#define UNCONSTRAINED_ENUMERATOR_CPP

#include "multitree.hpp"

#include <bitset>
#include <cassert>
#include <vector>

#include "bits.hpp"

namespace terraces {

using bits::bitscan;
using bits::popcount;
using bits::rbitscan;

struct small_bipartition {
	index_t m_mask;
	index_t m_cur_bip;

	small_bipartition(index_t mask = 1) : m_mask{mask} { reset(); }

	// credit goes to nglee
	// (see stackoverflow.com/questions/44767080/incrementing-masked-bitsets)
	index_t masked_increment(index_t bip) const { return -(bip ^ m_mask) & m_mask; }

	bool has_choices() const { return num_leaves() > 2; }

	bool is_valid() const { return (m_cur_bip >> rbitscan(m_mask)) == 0; }

	bool next() {
		assert(is_valid());
		m_cur_bip = masked_increment(m_cur_bip);
		return is_valid();
	}
	void reset() { m_cur_bip = index_t(1) << bitscan(m_mask); }

	index_t mask() const { return m_mask; }
	index_t left_mask() const { return m_cur_bip; }
	index_t right_mask() const { return m_cur_bip ^ m_mask; }
	index_t leftmost_leaf() const { return bitscan(m_mask); }
	index_t rightmost_leaf() const { return rbitscan(m_mask); }
	index_t num_leaves() const { return popcount(m_mask); }

	static small_bipartition full_set(index_t num_leaves) {
		assert(num_leaves < bits::word_bits);
		return {(index_t(1) << num_leaves) - 1};
	}
};
} // namespace terraces

#endif // UNCONSTRAINED_ENUMERATOR_CPP
