#include "multitree_iterator.hpp"

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
	init_subtree_unconstrained(i, unconstrained);
}

bool multitree_iterator::init_subtree_unconstrained(index_t root,
                                                    multitree_nodes::unconstrained data) {
	auto init_size = m_init_stack.size();
	m_init_stack.emplace(root);
	while (m_init_stack.size() > init_size) {
		auto i = m_init_stack.top();
		m_init_stack.pop();
		const auto& bip = m_unconstrained_choices[i];
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
			m_init_stack.push(left);
			m_init_stack.push(right);
		}
	}
	return true;
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
	m_init_stack.push(lindex);
	m_init_stack.push(rindex);
}

bool multitree_iterator::init_subtree(index_t root) {
	m_init_stack.push(root);
	while (!m_init_stack.empty()) {
		auto i = m_init_stack.top();
		m_init_stack.pop();
		const auto mt_node = m_choices[i].current;
		switch (mt_node->type) {
		case multitree_node_type::base_single_leaf:
			init_subtree(i, mt_node->single_leaf);
			break;
		case multitree_node_type::base_two_leaves:
			init_subtree(i, mt_node->two_leaves);
			break;
		case multitree_node_type::base_unconstrained:
			init_subtree(i, mt_node->unconstrained);
			break;
		case multitree_node_type::inner_node:
			init_subtree(i, mt_node->inner_node);
			break;
		case multitree_node_type::alternative_array:
			assert(false && "Malformed multitree: Nested alternative_arrays");
			break;
		case multitree_node_type::unexplored:
			throw multitree_unexplored_error{};
		}
	}
	return true;
}

bool multitree_iterator::next(index_t root) {
	auto node = m_tree[root];
	auto left = node.lchild();
	auto right = node.rchild();
	auto& choice = m_choices[root];
	switch (choice.current->type) {
	case multitree_node_type::base_single_leaf:
	case multitree_node_type::base_two_leaves:
		return false;
	case multitree_node_type::base_unconstrained:
		return next_unconstrained(root, choice.current->unconstrained);
	case multitree_node_type::inner_node:
	case multitree_node_type::alternative_array:
		return next(left) || (next(right) && reset(left)) ||
		       (choice.has_choices() && choice.next() && init_subtree(root));
	case multitree_node_type::unexplored:
		throw multitree_unexplored_error{};
	default:
		assert(false && "Unknown node type in multitree");
		return false;
	}
}

bool multitree_iterator::next_unconstrained(index_t root, multitree_nodes::unconstrained data) {
	auto node = m_tree[root];
	auto left = node.lchild();
	auto right = node.rchild();
	auto& choice = m_unconstrained_choices[root];
	if (!choice.has_choices()) {
		return false;
	}
	return next_unconstrained(left, data) ||
	       (next_unconstrained(right, data) && reset_unconstrained(left, data)) ||
	       (choice.next() && init_subtree_unconstrained(root, data));
}

bool multitree_iterator::reset(index_t root) {
	auto& choice = m_choices[root];
	if (choice.has_choices()) {
		choice.reset();
	}
	switch (choice.current->type) {
	case multitree_node_type::base_single_leaf:
	case multitree_node_type::base_two_leaves:
		break;
	case multitree_node_type::base_unconstrained:
		reset_unconstrained(root, choice.current->unconstrained);
		break;
	case multitree_node_type::inner_node:
	case multitree_node_type::alternative_array:
		init_subtree(root);
		break;
	case multitree_node_type::unexplored:
		throw multitree_unexplored_error{};
	default:
		assert(false && "Unknown node type in multitree");
		break;
	}
	return true;
}

bool multitree_iterator::reset_unconstrained(index_t root, multitree_nodes::unconstrained data) {
	auto& choice = m_unconstrained_choices[root];
	if (choice.has_choices()) {
		choice.reset();
	}
	init_subtree_unconstrained(root, data);
	return true;
}

bool multitree_iterator::next() { return next(0); }

} // namespace terraces
