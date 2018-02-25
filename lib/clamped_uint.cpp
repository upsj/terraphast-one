#include <terraces/clamped_uint.hpp>

#include <ostream>

#include "intrinsics.hpp"

namespace terraces {

namespace {
static constexpr auto max_index = std::numeric_limits<index>::max();
}

clamped_uint::clamped_uint(index value) : m_value{value} {}

clamped_uint& clamped_uint::operator+=(clamped_uint other) {
	if (bits::add_overflow(m_value, other.m_value, m_value)) {
		m_value = max_index;
	}
	return *this;
}

clamped_uint& clamped_uint::operator*=(clamped_uint other) {
	if (bits::mul_overflow(m_value, other.m_value, m_value)) {
		m_value = max_index;
	}
	return *this;
}

bool clamped_uint::is_clamped() const { return m_value == max_index; }

index clamped_uint::value() const { return m_value; }

clamped_uint operator+(clamped_uint a, clamped_uint b) { return a += b; }

clamped_uint operator*(clamped_uint a, clamped_uint b) { return a *= b; }

std::ostream& operator<<(std::ostream& stream, clamped_uint val) {
	if (val.is_clamped()) {
		stream << ">= ";
	}
	stream << val.value();
	return stream;
}

} // namespace terraces