#include <bitset>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>

#include <terraces/advanced.hpp>
#include <terraces/bigint.hpp>
#include <terraces/errors.hpp>
#include <terraces/parser.hpp>
#include <terraces/rooting.hpp>

#include "../lib/supertree_enumerator.hpp"
#include "../lib/supertree_variants.hpp"
#include "../lib/supertree_variants_debug.hpp"

using terraces::tree_enumerator;
using terraces::variants::count_callback;
using terraces::debug::variants::stack_state_decorator;

int main(int argc, char** argv) try {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <tree-file> <occurrence file>" << std::endl;
		return 1;
	}
	auto tree_file = std::ifstream{argv[1]};
	auto tree_string = std::string{};

	auto data_file = std::ifstream{argv[2]};
	const auto bitmatrix = terraces::parse_bitmatrix(data_file);

	std::getline(tree_file, tree_string);
	auto tree = terraces::parse_nwk(tree_string, bitmatrix.indices);

	terraces::utils::ensure<terraces::no_usable_root_error>(
	        bitmatrix.comp_taxon != terraces::none, "no usable root found");
	terraces::reroot_inplace(tree, bitmatrix.comp_taxon);

	auto data = terraces::prepare_constraints(tree, bitmatrix.matrix, bitmatrix.comp_taxon);
	omp_set_num_threads(6);
	omp_set_nested(1);
	tree_enumerator<stack_state_decorator<count_callback<mpz_class>>> enumerator{{}};

	std::thread status_thread([&]() {
		using namespace std::chrono_literals;
		auto& stack = enumerator.callback().stack(0);
		while (stack.empty()) {
			std::this_thread::sleep_for(100ms);
		}
		while (!stack.empty()) {
			std::this_thread::sleep_for(100ms);
			std::cout << "\x1B[2J\x1B[H";
			for (int t = 0; t < 6; ++t) {
				for (auto entry : enumerator.callback().stack(t)) {
					std::cout << (entry.right ? 'R' : 'L');
				}
				std::cout << std::endl;
			}
		}
	});

#pragma omp parallel
#pragma omp single
	{ std::cout << enumerator.run(data.num_leaves, data.constraints, data.root); }

} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << "\n";
}
