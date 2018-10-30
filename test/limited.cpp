#include <catch.hpp>

#include <algorithm>
#include <sstream>

#include <terraces/advanced.hpp>
#include <terraces/parser.hpp>
#include <terraces/rooting.hpp>
#include <terraces/subtree_extraction.hpp>
#include <thread>

#include "../lib/multitree_iterator.hpp"
#include "../lib/supertree_enumerator.hpp"
#include "../lib/supertree_variants_multitree.hpp"
#include "../lib/validation.hpp"

namespace terraces {
namespace tests {

TEST_CASE("", "") {
	auto magic = 12u;
	name_map nums{"root"};
	index_map indx{{"root", 0}};
	/**
	 * Construct 12 nested three-taxon trees
	 * that create many bipartitions in the first recursion step.
	 */
	std::stringstream nwk;
	nwk << "(root,(";
	for (unsigned i = 0; i < 3 * magic; i += 3) {
		nums.push_back(std::to_string(i));
		indx.emplace(nums.back(), i + 1);
		nums.push_back(std::to_string(i + 1));
		indx.emplace(nums.back(), i + 2);
		nums.push_back(std::to_string(i + 2));
		indx.emplace(nums.back(), i + 3);
		nwk << "((" << i << ',' << (i + 1) << ")," << (i + 2) << ')';
		nwk << (i < 3 * (magic - 2) ? ",(" : (i < 3 * (magic - 1) ? "," : ""));
	}
	for (unsigned i = 0; i < magic; ++i) {
		nwk << ')';
	}
	auto tree = parse_nwk(nwk.str(), indx);
	bitmatrix matrix{3 * magic + 1, magic};
	for (unsigned i = 0; i < 3 * magic; ++i) {
		matrix.set(i + 1, i / 3, 1);
	}
	for (unsigned i = 0; i < magic; ++i) {
		matrix.set(0, i, 1);
	}
	auto d = create_supertree_data(tree, matrix);
	SECTION("time-limit") {
		using cb = variants::timeout_decorator<variants::count_callback<index>>;
		tree_enumerator<cb> enumerator{cb{1}};
		SECTION("yes") {
			// artificially hit time limit
			std::this_thread::sleep_for(std::chrono::seconds(2));
			enumerator.run(d.num_leaves, d.constraints, d.root);
			REQUIRE(enumerator.callback().has_timed_out());
		}
		SECTION("no") {
			// no constraints -> straight to base case
			enumerator.run(d.num_leaves, {}, d.root);
			REQUIRE(!enumerator.callback().has_timed_out());
		}
	}
	SECTION("memory-limit") {
		using cb = variants::memory_limited_multitree_callback;
		tree_enumerator<cb> enumerator{cb{1 << 20}};
		SECTION("yes") {
			// constraints create many bipartitions in first recursion level -> limit
			enumerator.run(d.num_leaves, d.constraints, d.root);
			REQUIRE(enumerator.callback().has_hit_memory_limit());
		}
		SECTION("no") {
			// no constraints -> straight to base case
			enumerator.run(d.num_leaves, {}, d.root);
			REQUIRE(!enumerator.callback().has_hit_memory_limit());
		}
	}
}

} // namespace tests
} // namespace terraces
