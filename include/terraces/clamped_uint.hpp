#ifndef TERRACES_CLAMPED_UINT_HPP
#define TERRACES_CLAMPED_UINT_HPP

#include "definitions.hpp"

#include <iosfwd>

namespace terraces {

class clamped_uint {
	index m_value;

public:
	clamped_uint(index value = 0);

	clamped_uint& operator+=(clamped_uint other);
	clamped_uint& operator*=(clamped_uint other);
	bool is_clamped() const;
	index value() const;
};

clamped_uint operator+(clamped_uint a, clamped_uint b);

clamped_uint operator*(clamped_uint a, clamped_uint b);

std::ostream& operator<<(std::ostream& stream, clamped_uint val);

} // namespace terraces

#endif // TERRACES_CLAMPED_UINT_HPP
