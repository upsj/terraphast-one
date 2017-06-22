#include <catch.hpp>

#include <terraces/bitvector.hpp>

namespace terraces {
namespace test {

TEST_CASE("fast_set1", "[fast_set]") {
	bitvector set{10};
	for (auto&& el : set) {
		(void)el;
		FAIL("element in empty set!");
	}
	set.set(4);
	set.update_ranks();
	CHECK(*set.begin() == 4);
	CHECK((++set.begin()) == set.end());
	CHECK(set.get(4));
	CHECK(!set.get(5));
	set.clr(4);
	set.set(5);
	set.update_ranks();
	CHECK(!set.get(4));
	CHECK(set.get(5));
	CHECK(*set.begin() == 5);
	CHECK((++set.begin()) == set.end());
	set.set(2);
	set.update_ranks();
	index count = 0;
	for (auto&& el : set) {
		(void)el;
		++count;
	}
	CHECK(count == set.count());
}

} // namespace test
} // namespace terraces
