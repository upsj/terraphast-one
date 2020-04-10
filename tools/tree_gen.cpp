#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

using index_t = std::size_t;
constexpr static auto none = std::numeric_limits<index_t>::max();

inline std::default_random_engine& get_rng() {
	thread_local auto rng = [] {
		const auto seed = std::random_device{}();
		return std::default_random_engine{seed};
	}();
	return rng;
}

inline index_t rand_index(index_t max) {
	auto dist = std::uniform_int_distribution<index_t>{0u, max};
	return dist(get_rng());
}

struct tree_node {
	explicit tree_node(index_t parent = none, index_t left = none, index_t right = none)
	        : parent{parent}, left{left}, right{right} {}
	index_t parent = none;
	index_t left = none;
	index_t right = none;
	std::uint8_t visited = 0u;

	void set_children(index_t l, index_t r) {
		left = l;
		right = r;
	}

	bool is_leaf() const { return left == none and right == none; }
};

using tree = std::vector<tree_node>;
using index_list = std::vector<index_t>;

inline index_t extract_random(index_list& list) {
	const auto i = rand_index(list.size() - 1u);
	const auto ret = list[i];
	list[i] = list.back();
	list.pop_back();
	return ret;
}

inline void print(std::ostream& out, tree& t, const std::vector<std::string>& names) {
	auto current = index_t{};
	while (current != none) {
		auto& cur = t[current];
		switch (cur.visited) {
		case 0:
			if (cur.is_leaf()) {
				out << names[current];
				current = cur.parent;
			} else {
				out << "(";
				current = cur.left;
			}
			break;
		case 1:
			out << ", ";
			current = cur.right;
			break;
		case 2:
			out << ')' << names[current];
			current = cur.parent;
			break;
		default:
			throw std::logic_error{"WAT?"};
		}
		++cur.visited;
	}
}

int main(int argc, char** argv) try {
	if (argc < 2) {
		return 1;
	}
	const auto num_leafs = index_t{std::stoul(argv[1])};
	if (num_leafs < 2) {
		return 2;
	}
	auto t = tree{tree_node{none, none, none}};
	auto leafs = index_list{0u};
	for (auto i = 1u; i < num_leafs; ++i) {
		const auto parent = extract_random(leafs);
		const auto lchild = t.size();
		t.emplace_back(parent);
		leafs.emplace_back(lchild);
		const auto rchild = t.size();
		t.emplace_back(parent);
		leafs.emplace_back(rchild);
		t[parent].set_children(lchild, rchild);
	}
	auto names = std::vector<std::string>(t.size());
	for (auto i = index_t{}; i < leafs.size(); ++i) {
		names[leafs[i]] = "s" + std::to_string(i);
	}
	print(std::cout, t, names);
	std::cout << '\n';
} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << '\n';
	return 42;
}
