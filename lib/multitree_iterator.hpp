#ifndef MULTITREE_ITERATOR_H
#define MULTITREE_ITERATOR_H

#include <terraces/errors.hpp>

#include "multitree.hpp"
#include "small_bipartition.hpp"

namespace terraces {

struct multitree_iterator_choicepoint {
	const multitree_node* alternatives;
	const multitree_node* current;

	multitree_iterator_choicepoint() : alternatives{nullptr}, current{nullptr} {}

	multitree_iterator_choicepoint(const multitree_node* node) {
		if (node->type == multitree_node_type::alternative_array) {
			const auto aa = node->alternative_array;
			alternatives = aa.num_alternatives() > 1 ? node : nullptr;
			current = aa.begin;
		} else {
			alternatives = nullptr;
			current = node;
		}
	}

	bool has_choices() const { return alternatives != nullptr; }

	bool is_unconstrained() const {
		return current->type == multitree_node_type::base_unconstrained;
	}

	bool is_valid() const {
		return has_choices() && current != alternatives->alternative_array.end;
	}

	bool next() {
		assert(is_valid());
		++current;
		return is_valid();
	}

	void reset() { current = alternatives->alternative_array.begin; }
};

class multitree_iterator {
private:
	terraces::tree m_tree;
	std::vector<multitree_iterator_choicepoint> m_choices;
	std::vector<small_bipartition> m_unconstrained_choices;
	multitree_nodes::unconstrained m_unconstrained_current{};
	struct state_t {
		index_t root;
		int state{};
		bool result{};
		bool unconstrained{};
		state_t(index_t root, bool unconstrained)
		        : root{root}, unconstrained{unconstrained} {}
	};
	std::stack<state_t> m_stack;
	std::stack<index_t> m_init_stack;

	void init_subtree(index_t subtree_root);
	void init_subtree(index_t subtree_root, index_t single_leaf);
	void init_subtree(index_t subtree_root, multitree_nodes::two_leaves two_leaves);
	void init_subtree(index_t subtree_root, multitree_nodes::inner_node inner);
	void init_subtree(index_t subtree_root, multitree_nodes::unconstrained unconstrained);
	void init_subtree_unconstrained(index_t subtree_root);

	bool next(index_t root);
	bool next_unconstrained(index_t root);
	void reset(index_t root);
	void reset_unconstrained(index_t root);

public:
	multitree_iterator(const multitree_node* root);
	bool next();
	const terraces::tree& tree() const { return m_tree; }
};

} // namespace terraces

#endif // MULTITREE_ITERATOR_H
