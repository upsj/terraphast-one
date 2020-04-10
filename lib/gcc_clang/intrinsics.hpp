#include <terraces/definitions.hpp>

namespace terraces {
namespace bits {

inline index_t popcount(index_t word) { return index_t(__builtin_popcountll(word)); }

inline index_t bitscan(index_t word) { return index_t(__builtin_ctzll(word)); }

static_assert(sizeof(long long) >= sizeof(index_t), "intrinsic word sizes incompatible");

inline index_t rbitscan(index_t word) {
	return index_t(std::numeric_limits<long long>::digits - __builtin_clzll(word));
}

inline bool add_overflow(index_t a, index_t b, index_t& result) {
	return __builtin_add_overflow(a, b, &result);
}

inline bool mul_overflow(index_t a, index_t b, index_t& result) {
	return __builtin_mul_overflow(a, b, &result);
}

} // namespace bits
} // namespace terraces
