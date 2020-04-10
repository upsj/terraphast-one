#ifndef BITS_HPP
#define BITS_HPP

#include <intrinsics.hpp>

namespace terraces {
namespace bits {

static_assert(std::numeric_limits<index_t>::radix == 2, "Our integers must be of base 2");
constexpr index_t word_bits = std::numeric_limits<index_t>::digits;

inline index_t block_index(index_t i) { return i / word_bits; }
inline index_t base_index(index_t block) { return block * word_bits; }
inline index_t shift_index(index_t i) { return i % word_bits; }
inline index_t set_mask(index_t i) { return index_t(1) << (i & (word_bits - 1)); }
inline index_t clear_mask(index_t i) { return ~set_mask(i); }
inline index_t prefix_mask(index_t i) { return set_mask(i) - 1; }
inline index_t next_bit(index_t block, index_t i) { return i + bitscan(block >> shift_index(i)); }
inline index_t next_bit0(index_t block, index_t i) { return i + bitscan(block); }
inline bool has_next_bit(index_t block, index_t i) { return (block >> shift_index(i)) != 0; }
inline bool has_next_bit0(index_t block) { return block != 0; }
inline index_t partial_popcount(index_t block, index_t i) {
	return popcount(block & prefix_mask(i));
}

} // namespace bits
} // namespace terraces

#endif // BITS_HPP
