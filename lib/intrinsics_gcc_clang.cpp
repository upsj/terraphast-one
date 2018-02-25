#include "intrinsics.hpp"

namespace terraces {
namespace bits {

index popcount(index word) { return (index)__builtin_popcountll(word); }

index bitscan(index word) { return (index)__builtin_ctzll(word); }

index rbitscan(index word) { return (index)(63 - __builtin_clzll(word)); }

bool add_overflow(index a, index b, index& result) { return __builtin_add_overflow(a, b, &result); }

bool mul_overflow(index a, index b, index& result) { return __builtin_mul_overflow(a, b, &result); }

} // namespace bits
} // namespace terraces
