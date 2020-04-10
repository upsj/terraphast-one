#ifndef TERRACES_BIPARTITIONS_HPP
#define TERRACES_BIPARTITIONS_HPP

#include <cmath>
#include <iosfwd>

#include <terraces/trees.hpp>

#include "ranked_bitvector.hpp"
#include "union_find.hpp"

namespace terraces {

/**
 * An iterator enumerating all possible bipartition from a given union-find representation of leaf
 * sets.
 */
class bipartitions {
private:
	utils::stack_allocator<index_t> m_alloc;
	const ranked_bitvector& m_leaves;
	const union_find& m_sets;
	const ranked_bitvector m_set_rep;

	index_t m_end;

	bool in_left_partition(index_t bip, index_t i) const;
	/** Returns a bitvector containing a 1 for every set representative in the union-find
	 * structure. */
	ranked_bitvector find_set_reps() const;

public:
	bipartitions(const ranked_bitvector& leaves, const union_find& sets,
	             utils::stack_allocator<index_t>);
	/** Returns the first leaf set represented by the given bipartition index. */
	ranked_bitvector get_first_set(index_t bip, utils::stack_allocator<index_t> alloc) const;
	/** Replaces a leaf subset by its complement. */
	void flip_set(ranked_bitvector& set) const;
	/** Returns both leaf sets represented by the given bipartition index. */
	std::pair<ranked_bitvector, ranked_bitvector>
	get_both_sets(index_t bip, utils::stack_allocator<index_t> alloc) const;
	/**Returns both leaf sets represented by the given bipartition index.
	 * ONLY USE THIS METHOD IN SINGLE-THREADED EXECUTION! */
	std::pair<ranked_bitvector, ranked_bitvector> get_both_sets_unsafe(index_t bip) const;

	index_t begin_bip() const { return 1; }
	index_t end_bip() const { return m_end; }
	/** Returns the number of bipartitions. */
	index_t num_bip() const { return m_end - 1; }
	/** Returns the union-find representation of the leaf sets. */
	const union_find& sets() const { return m_sets; }
	/** Returns the leaves on which the union-find representation is based. */
	const ranked_bitvector& leaves() const { return m_leaves; }
};

} // namespace terraces

#endif // TERRACES_BIPARTITIONS_HPP
