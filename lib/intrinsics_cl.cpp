#include "intrinsics.hpp"

#pragma intrinsic(_BitScanForward64, __popcnt64)

namespace terraces {
namespace bits {

index popcount(index word) { return (index)__popcnt64(word); }

index bitscan(index word) {
	unsigned long idx;
	_BitScanForward64(&idx, word);
	return (index)idx;
}

index rbitscan(index word) {
	unsigned long idx;
	_BitScanReverse64(&idx, word);
	return (index)idx;
}

namespace {
constexpr index max_index = std::numeric_limits<index>::max();
}

bool add_overflow(index a, index b, index& result) {
	result = a + b;
	if (max_index - b < a) {
		return true;
	} else {
		return false;
	}
}

bool mul_overflow(index a, index b, index& result) {
	result = a * b;
	if (max_index / b < a) {
		return true;
	} else {
		return false;
	}
}

} // namespace bits
} // namespace terraces
