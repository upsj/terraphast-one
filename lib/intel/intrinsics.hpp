#include <terraces/definitions.hpp>

namespace terraces {
namespace bits {

#if defined(__LP64__) || defined(_LP64)
inline index_t popcount(index_t word) { return index_t(_popcnt64(word)); }

inline index_t bitscan(index_t word) {
	unsigned int idx;
	_BitScanForward64(&idx, word);
	return index_t(idx);
}

inline index_t rbitscan(index_t word) {
	unsigned int idx;
	_BitScanReverse64(&idx, word);
	return index_t(idx);
}
#else
inline index_t popcount(index_t word) { return index_t(_popcnt(word)); }

inline index_t bitscan(index_t word) {
	unsigned int idx;
	_BitScanForward(&idx, word);
	return index_t(idx);
}

inline index_t rbitscan(index_t word) {
	unsigned int idx;
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
