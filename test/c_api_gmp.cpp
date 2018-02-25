#include <catch.hpp>
#include <terraces/terraces.h>

#ifdef USE_GMP
#include <gmp.h>
TEST_CASE("c_api_count_str", "[c-api]") {
	mpz_t result;
	mpz_init(result);
	REQUIRE(terraces_count_tree_str(
	                "6 3\n1 0 0 s1\n1 0 0 s2\n0 0 1 s3\n0 0 1 s4\n1 1 1 s5\n0 1 1 s6",
	                "((s4, (s3, (s2, (s1, s6)))), s5)", result) == terraces_success);
	CHECK(mpz_get_ui(result) == 35);
}

TEST_CASE("c_api_count_data", "[c-api]") {
	mpz_t result;
	mpz_init(result);
	terraces_missing_data data;
	data.num_species = 6;
	data.num_partitions = 3;
	unsigned char matrix[] = {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1};
	const char* names[] = {"s1", "s2", "s3", "s4", "s5", "s6"};
	data.matrix = matrix;
	data.names =
	        const_cast<char**>(names); // circumvent ISO C++11 disallowing char* a = "literal"
	data.allocatedNameArray = true;
	REQUIRE(terraces_count_tree(&data, "((s4, (s3, (s2, (s1, s6)))), s5)", result) ==
	        terraces_success);
	CHECK(mpz_get_ui(result) == 35);
}
#endif