#include <catch.hpp>

#include <terraces/advanced.hpp>
#include <terraces/errors.hpp>

namespace terraces {
namespace tests {

TEST_CASE("maximum_comprehensive_columnset", "[advanced-api]") {
	auto matrix_full = bitmatrix{4, 4};
	matrix_full.set(0, 2, 1);
	matrix_full.set(1, 2, 1);
	matrix_full.set(1, 3, 1);
	matrix_full.set(2, 2, 1);
	matrix_full.set(3, 0, 1);
	auto matrix_reduced = bitmatrix{4, 2};
	matrix_reduced.set(0, 0, 1);
	matrix_reduced.set(1, 0, 1);
	matrix_reduced.set(1, 1, 1);
	matrix_reduced.set(2, 0, 1);
	auto matrix_result = maximum_comprehensive_columnset(matrix_full);
	CHECK(matrix_result.second == 1);
	CHECK(matrix_result.first == matrix_reduced);

	CHECK_THROWS_AS(maximum_comprehensive_columnset(bitmatrix{10, 10}), bad_input_error);
}

} // namespace tests
} // namespace terraces