#include "multitree_iterator.hpp"
#include "utils.hpp"

namespace terraces {

multitree_iterator::multitree_iterator(const multitree_node* root)
        : m_tree(2 * root->num_leaves - 1), m_choices(m_tree.size()),
          m_unconstrained_choices(m_tree.size()) {
	m_choices[0] = {root};
	init_subtree(0);
}

void multitree_iterator::init_subtree(index_t i, index_t single_leaf) {
	m_tree[i].lchild() = none;
	m_tree[i].rchild() = none;
	m_tree[i].taxon() = single_leaf;
}

void multitree_iterator::init_subtree(index_t i, multitree_nodes::two_leaves two_leaves) {
	const auto l = i + 1;
	const auto r = i + 2;
	m_tree[i].lchild() = l;
	m_tree[i].rchild() = r;
	m_tree[i].taxon() = none;
	m_tree[l] = {i, none, none, two_leaves.left_leaf};
	m_tree[r] = {i, none, none, two_leaves.right_leaf};
}

void multitree_iterator::init_subtree(index_t i, multitree_nodes::unconstrained unconstrained) {
	m_unconstrained_choices[i] = small_bipartition::full_set(unconstrained.num_leaves());
	m_unconstrained_current = unconstrained;
	init_subtree_unconstrained(i);
}

void multitree_iterator::init_subtree_unconstrained(index_t i) {
	const auto& bip = m_unconstrained_choices[i];
	const auto& data = m_unconstrained_current;
	auto& node = m_tree[i];
	if (bip.num_leaves() <= 2) {
		if (bip.num_leaves() == 1) {
			node.lchild() = none;
			node.rchild() = none;
			node.taxon() = data.begin[bip.leftmost_leaf()];
		} else {
			node.lchild() = i + 1;
			node.rchild() = i + 2;
			node.taxon() = none;
			m_tree[i + 1] = {i, none, none, data.begin[bip.leftmost_leaf()]};
			m_tree[i + 2] = {i, none, none, data.begin[bip.rightmost_leaf()]};
		}
	} else {
		const auto lbip = small_bipartition{bip.left_mask()};
		const auto rbip = small_bipartition{bip.right_mask()};
		const auto left = i + 1;
		const auto right = i + 1 + 2 * lbip.num_leaves() - 1;
		node.lchild() = left;
		node.rchild() = right;
		node.taxon() = none;
		m_unconstrained_choices[left] = lbip;
		m_unconstrained_choices[right] = rbip;
		m_tree[node.lchild()].parent() = i;
		m_tree[node.rchild()].parent() = i;

		init_subtree_unconstrained(left);
		init_subtree_unconstrained(right);
	}
}

void multitree_iterator::init_subtree(index_t i, multitree_nodes::inner_node inner) {
	const auto left = inner.left;
	const auto right = inner.right;
	const auto lindex = i + 1;
	const auto rindex = lindex + (2 * left->num_leaves - 1);
	m_tree[i].lchild() = lindex;
	m_tree[i].rchild() = rindex;
	m_tree[i].taxon() = none;
	m_tree[lindex].parent() = i;
	m_tree[rindex].parent() = i;
	m_choices[lindex] = {left};
	m_choices[rindex] = {right};
	init_subtree(lindex);
	init_subtree(rindex);
}

void multitree_iterator::init_subtree(index_t i) {
	const auto mt_node = m_choices[i].current;
	switch (mt_node->type) {
	case multitree_node_type::base_single_leaf:
		return init_subtree(i, mt_node->single_leaf);
	case multitree_node_type::base_two_leaves:
		return init_subtree(i, mt_node->two_leaves);
	case multitree_node_type::base_unconstrained:
		return init_subtree(i, mt_node->unconstrained);
	case multitree_node_type::inner_node:
		return init_subtree(i, mt_node->inner_node);
	case multitree_node_type::alternative_array:
		assert(false && "Malformed multitree: Nested alternative_arrays");
		break;
	case multitree_node_type::unexplored:
		throw multitree_unexplored_error{};
	}
}

#define RETURN_IMPL(ret, cond)                                                                     \
	{                                                                                          \
		auto res = ret;                                                                    \
		m_stack.pop();                                                                     \
		if (cond) {                                                                        \
			return res;                                                                \
		}                                                                                  \
		m_stack.top().result = res;                                                        \
		break;                                                                             \
	}

#define RETURN(ret) RETURN_IMPL(ret, m_stack.empty())

#define RETURN_UNCONSTRAINED(ret) RETURN_IMPL(ret, m_stack.empty() || !m_stack.top().unconstrained)

#define YIELD_IMPL(next_state)                                                                     \
	{                                                                                          \
		top.state = next_state;                                                            \
		break;                                                                             \
	}                                                                                          \
	/* fall through */                                                                         \
	case next_state:

#define YIELD YIELD_IMPL(__COUNTER__)

#define CALL(idx)                                                                                  \
	{ m_stack.emplace(idx, false); }                                                           \
	YIELD

#define CALL_UNCONSTRAINED(idx)                                                                    \
	{ m_stack.emplace(idx, true); }                                                            \
	YIELD

bool multitree_iterator::next(index_t root) {
	// make sure the counter is larger than zero
	(void)__COUNTER__;

	auto is_trivial = [&](index_t idx) {
		const auto& choice = m_choices[idx];
		const auto type = choice.current->type;
		utils::ensure<multitree_unexplored_error>(type != multitree_node_type::unexplored);
		return type == multitree_node_type::base_single_leaf ||
		       type == multitree_node_type::base_two_leaves;
	};
	if (is_trivial(root)) {
		return false;
	}
	m_stack.emplace(root, false);
	while (true) {
		auto& top = m_stack.top();
		const auto idx = top.root;
		const auto node = m_tree[idx];
		const auto left = node.lchild();
		const auto right = node.rchild();
		auto& choice = m_choices[idx];
		const auto type = choice.current->type;
		switch (top.state) {
		case 0:
			if (type == multitree_node_type::base_unconstrained) {
				if (!next_unconstrained(idx)) {
					m_unconstrained_current = {};
					RETURN(false);
				} else {
					RETURN(true);
				}
			}

			if (!is_trivial(left)) {
				CALL(left);
				if (top.result) {
					RETURN(true);
				}
			}

			if (!is_trivial(right)) {
				CALL(right);
				if (top.result) {
					reset(left);
					RETURN(true);
				}
			}

			if (choice.has_choices() && choice.next()) {
				init_subtree(idx);
				RETURN(true);
			}
			RETURN(false);
		default:;
		}
	}
}

bool multitree_iterator::next_unconstrained(index_t root) {
	auto& choice = m_unconstrained_choices[root];
	if (!choice.has_choices()) {
		return false;
	}
	m_stack.emplace(root, true);
	while (true) {
		auto& top = m_stack.top();
		const auto idx = top.root;
		const auto cur = m_tree[idx];
		auto& choice = m_unconstrained_choices[idx];

		const auto left = cur.lchild();
		const auto right = cur.rchild();
		switch (top.state) {
		case 0:
			if (m_unconstrained_choices[left].has_choices()) {
				CALL_UNCONSTRAINED(left);
				if (top.result) {
					RETURN_UNCONSTRAINED(true);
				}
			}
			if (m_unconstrained_choices[right].has_choices()) {
				CALL_UNCONSTRAINED(right);
				if (top.result) {
					reset_unconstrained(left);
					RETURN_UNCONSTRAINED(true);
				}
			}
			if (choice.next()) {
				init_subtree_unconstrained(idx);
				RETURN_UNCONSTRAINED(true);
			}
			RETURN_UNCONSTRAINED(false);
		default:;
		}
	}
}

void multitree_iterator::reset(index_t root) {
	auto& choice = m_choices[root];
	if (choice.has_choices()) {
		choice.reset();
	}
	switch (choice.current->type) {
	case multitree_node_type::base_single_leaf:
	case multitree_node_type::base_two_leaves:
		return;
	case multitree_node_type::base_unconstrained:
		return reset_unconstrained(root);
	case multitree_node_type::inner_node:
	case multitree_node_type::alternative_array:
		return init_subtree(root);
	case multitree_node_type::unexplored: {
		throw multitree_unexplored_error{};
	}
	default:
		assert(false && "Unknown node type in multitree");
		return;
	}
}

void multitree_iterator::reset_unconstrained(index_t root) {
	auto& choice = m_unconstrained_choices[root];
	if (choice.has_choices()) {
		choice.reset();
	}
	init_subtree_unconstrained(root);
}

bool multitree_iterator::next() { return next(0); }

} // namespace terraces
