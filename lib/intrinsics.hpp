#ifndef TERRAPHAST_INTRINSICS_HPP
#define TERRAPHAST_INTRINSICS_HPP

#include <terraces/trees.hpp>

namespace terraces {
namespace bits {

index popcount(index word);

index bitscan(index word);

index rbitscan(index word);

bool add_overflow(index a, index b, index& result);

bool mul_overflow(index a, index b, index& result);

} // namespace bits
} // namespace terraces

#endif // TERRAPHAST_INTRINSICS_HPP
