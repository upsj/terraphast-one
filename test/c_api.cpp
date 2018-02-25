#include <catch.hpp>
#include <terraces/terraces.h>

TEST_CASE("c_api_non_terrace_str", "[c-api]") {
	bool result = false;
	REQUIRE(terraces_check_tree_str("4 2\n1 0 s1\n1 1 s2\n1 1 s3\n1 1 s4",
	                                "(s1, (s2, (s3, s4)))", &result) == terraces_success);
	CHECK(!result);
}

TEST_CASE("c_api_terrace_str", "[c-api]") {
	bool result = false;
	REQUIRE(terraces_check_tree_str("5 2\n1 0 s1\n1 0 s2\n1 1 s3\n0 1 s4\n0 1 s5",
	                                "(s3, ((s1, s2), (s4, s5)))", &result) == terraces_success);
	CHECK(result);
}

TEST_CASE("c_api_non_terrace_data", "[c-api]") {
	bool result;
	terraces_missing_data data;
	data.num_species = 4;
	data.num_partitions = 2;
	unsigned char matrix[] = {1, 0, 1, 1, 1, 1, 1, 1};
	const char* names[] = {"s1", "s2", "s3", "s4"};
	data.matrix = matrix;
	data.names =
	        const_cast<char**>(names); // circumvent ISO C++11 disallowing char* a = "literal"
	data.allocatedNameArray = true;
	REQUIRE(terraces_check_tree(&data, "(s1, (s2, (s3, s4)))", &result) == terraces_success);
	CHECK(!result);
}

TEST_CASE("c_api_terrace_data", "[c-api]") {
	bool result;
	terraces_missing_data data;
	data.num_species = 5;
	data.num_partitions = 2;
	unsigned char matrix[] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1};
	const char* names[] = {"s1", "s2", "s3", "s4", "s5"};
	data.matrix = matrix;
	data.names =
	        const_cast<char**>(names); // circumvent ISO C++11 disallowing char* a = "literal"
	data.allocatedNameArray = true;
	REQUIRE(terraces_check_tree(&data, "(s3, ((s1, s2), (s4, s5)))", &result) ==
	        terraces_success);
	CHECK(result);
}