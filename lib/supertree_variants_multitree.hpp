#ifndef SUPERTREE_VARIANTS_MULTITREE_HPP
#define SUPERTREE_VARIANTS_MULTITREE_HPP

#include "multitree_impl.hpp"
#include "supertree_variants.hpp"

#include <memory>
#include <stack>

namespace terraces {
namespace variants {

class multitree_callback : public abstract_callback<multitree_node*> {
private:
	friend class memory_limited_multitree_callback;
	multitree_impl::storage_blocks<multitree_node> m_nodes;
	multitree_impl::storage_blocks<index_t> m_leaves;

	multitree_node* alloc_node() { return m_nodes.get(); }

	multitree_node* alloc_nodes(index_t num) { return m_nodes.get_range(num); }

	std::pair<index_t*, index_t*> alloc_leaves(const ranked_bitvector& leaves) {
		auto size = leaves.count();
		auto a_leaves = m_leaves.get_range(size);
		index_t i = 0;
		for (auto el : leaves) {
			a_leaves[i++] = el;
		}
		return {a_leaves, a_leaves + size};
	}

protected:
	void set_node_memory_limit(index_t limit) { m_nodes.set_memory_limit(limit); }

public:
	using return_type = multitree_node*;

	return_type base_one_leaf(index_t i) {
		return multitree_impl::make_single_leaf(alloc_node(), i);
	}
	return_type base_two_leaves(index_t i, index_t j) {
		return multitree_impl::make_two_leaves(alloc_node(), i, j);
	}
	return_type base_unconstrained(const ranked_bitvector& leaves) {
		return multitree_impl::make_unconstrained(alloc_node(), alloc_leaves(leaves));
	}
	return_type null_result() const { return nullptr; }

	return_type fast_return_value(const bipartitions& bip_it) {
		return multitree_impl::make_unexplored(alloc_node(), alloc_leaves(bip_it.leaves()));
	}

	return_type begin_iteration(const bipartitions& bip_it, const bitvector&,
	                            const constraints&) {
		auto new_node = alloc_node();
		try {
			// alloc_nodes can fail for huge bip_ip.num_bip()
			// in this case, we return an unexplored node instead of failing completely
			auto alternatives = alloc_nodes(bip_it.num_bip());
			return multitree_impl::make_alternative_array(new_node, alternatives,
			                                              bip_it.leaves().count());
		} catch (std::bad_alloc&) {
			return multitree_impl::make_unexplored(new_node,
			                                       alloc_leaves(bip_it.leaves()));
		}
	}

	return_type accumulate(multitree_node* acc, multitree_node* node) {
		assert(acc->num_leaves == node->num_leaves);
		acc->num_trees += node->num_trees;
		*(acc->alternative_array.end) = *node;
		++(acc->alternative_array.end);
		return acc;
	}

	return_type combine(multitree_node* left, multitree_node* right) {
		return multitree_impl::make_inner_node(alloc_node(), left, right);
	}
};

class memory_limited_multitree_callback : public multitree_callback {
private:
	index_t m_memory_limit;
	bool m_hit_memory_limit;

	bool check_memory_limit() {
		auto memory = m_leaves.total_size() + m_nodes.total_size();
		if (memory > m_memory_limit) {
			m_hit_memory_limit = true;
		}
		return m_hit_memory_limit;
	}

public:
	memory_limited_multitree_callback(index_t limit)
	        : m_memory_limit(limit), m_hit_memory_limit{false} {
		// this is only a rough upper bound, but the number of leaves should be much below
		// the number of nodes in the multitree.
		set_node_memory_limit(limit);
	}

	bool fast_return(const bipartitions& bip_it) {
		return multitree_callback::fast_return(bip_it) || check_memory_limit();
	}

	bool continue_iteration(result_type acc) {
		return multitree_callback::continue_iteration(acc) && !check_memory_limit();
	}

	bool has_hit_memory_limit() const { return m_hit_memory_limit; }
};

} // namespace variants
} // namespace terraces

#endif // SUPERTREE_VARIANTS_MULTITREE_HPP
