#include <terraces/definitions.hpp>

#include <intrin.h>

#ifdef _WIN64
#pragma intrinsic(_BitScanForward64, _BitScanReverse64, __popcnt64)
#else
#pragma intrinsic(_BitScanForward, _BitScanReverse, __popcnt)
#endif

namespace terraces {
namespace bits {

#ifdef _WIN64
inline index_t popcount(index_t word) { return (index_t)__popcnt64(word); }

inline index_t bitscan(index_t word) {
	unsigned long idx;
	_BitScanForward64(&idx, word);
	return (index_t)idx;
}

inline index_t rbitscan(index_t word) {
	unsigned long idx;
	_BitScanReverse64(&idx, word);
	return (index_t)idx;
}
#else
inline index_t popcount(index_t word) { return index_t(__popcnt(word)); }

inline index_t bitscan(index_t word) {
	unsigned long idx;
	_BitScanForward(&idx, word);
	return index_t(idx);
}

inline index_t rbitscan(index_t word) {
	unsigned long idx;
	_BitScanReverse(&idx, word);
	return index_t(idx);
}
#endif

namespace {
constexpr index_t max_index = std::numeric_limits<index_t>::max();
}

inline bool add_overflow(index_t a, index_t b, index_t& result) {
	result = a + b;
	if (max_index - b < a) {
		return true;
	} else {
		return false;
	}
}

inline bool mul_overflow(index_t a, index_t b, index_t& result) {
	result = a * b;
	if (b != 0 && max_index / b < a) {
		return true;
	} else {
		return false;
	}
}

} // namespace bits
} // namespace terraces
