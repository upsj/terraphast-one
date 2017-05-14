
#include <catch.hpp>

#include <terraces/bitmatrix.hpp>


TEST_CASE("bitmatrix-construction","[bitmatrix]") {
	auto mat = terraces::bitmatrix{10, 5};
	CHECK(mat.rows() == 10);
	CHECK(mat.cols() == 5);
}

TEST_CASE("bitmatrix set/get", "[bitmatrix]") {
	auto mat = terraces::bitmatrix{3, 4};

	CHECK(mat.get(1,2) == false);
	mat.set(1, 2, true);
	CHECK(mat.get(1,2) == true);
}
