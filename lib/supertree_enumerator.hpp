#ifndef SUPERTREE_ENUMERATOR_HPP
#define SUPERTREE_ENUMERATOR_HPP

#include <atomic>
#include <omp.h>
#include <thread>

#include "bipartitions.hpp"
#include "stack_allocator.hpp"
#include "union_find.hpp"

#include "supertree_helpers.hpp"
#include "utils.hpp"

namespace terraces {

template <typename Callback>
class tree_enumerator {
	using result_type = typename Callback::result_type;

private:
	Callback m_cb;

	std::vector<utils::free_list> m_fl1;
	std::vector<utils::free_list> m_fl2;
	std::vector<utils::free_list> m_fl3;
	index m_fl1_allocsize;
	index m_fl2_allocsize;
	index m_fl3_allocsize;

	const constraints* m_constraints;

	result_type run(const ranked_bitvector& leaves, const bitvector& constraint_occ);
	result_type iterate(bipartitions& bip_it, const bitvector& new_constraint_occ);
	result_type iterate_parallel(bipartitions& bip_it, const bitvector& new_constraint_occ);
	result_type iterate_sequential(bipartitions& bip_it, const bitvector& new_constraint_occ);

	void init_freelists(index leaf_count, index constraint_count);
	index threadnum() const;
	utils::stack_allocator<index> leaf_allocator();
	utils::stack_allocator<index> c_occ_allocator();
	utils::stack_allocator<index> union_find_allocator();

public:
	explicit tree_enumerator(Callback cb);
	result_type run(index num_leaves, const constraints& constraints, index root_leaf);
	result_type run(index num_leaves, const constraints& constraints);
	const Callback& callback() const { return m_cb; }
};

template <typename Callback>
tree_enumerator<Callback>::tree_enumerator(Callback cb)
        : m_cb{std::move(cb)}, m_constraints{nullptr} {}

template <typename Callback>
auto tree_enumerator<Callback>::run(index num_leaves, const constraints& constraints)
        -> result_type {
	init_freelists(num_leaves, constraints.size());
	auto leaves = full_ranked_set(num_leaves, leaf_allocator());
	auto c_occ = full_set(constraints.size(), c_occ_allocator());
	assert(filter_constraints(leaves, c_occ, constraints, c_occ_allocator()) == c_occ);
	m_constraints = &constraints;
	return run(leaves, c_occ);
}

template <typename Callback>
auto tree_enumerator<Callback>::run(index num_leaves, const constraints& constraints,
                                    index root_leaf) -> result_type {
	init_freelists(num_leaves, constraints.size());
	auto leaves = full_ranked_set(num_leaves, leaf_allocator());
	auto c_occ = full_set(constraints.size(), c_occ_allocator());
	assert(filter_constraints(leaves, c_occ, constraints, c_occ_allocator()) == c_occ);
	// enter the call
	m_cb.enter(leaves);
	// no base cases
	assert(num_leaves > 2);
	assert(!constraints.empty());
	// build bipartition iterator:
	auto sets = union_find{num_leaves, union_find_allocator()};
	index rep = root_leaf == 0 ? 1 : 0;
	// merge all non-root leaves into one set
	for (index i = rep + 1; i < num_leaves; ++i) {
		if (i != root_leaf) {
			sets.merge(rep, i);
		}
	}
	sets.compress();
	m_constraints = &constraints;
	auto bip_it = bipartitions{leaves, sets, leaf_allocator()};
	return m_cb.exit(iterate(bip_it, c_occ));
}

template <typename Callback>
auto tree_enumerator<Callback>::run(const ranked_bitvector& leaves, const bitvector& constraint_occ)
        -> result_type {
	//#pragma omp critical
	m_cb.enter(leaves);

	// base cases: only a few leaves
	assert(leaves.count() > 0);
	if (leaves.count() == 1) {
		auto result = m_cb.null_result();
		//#pragma omp critical
		result = m_cb.exit(m_cb.base_one_leaf(leaves.first_set()));
		return result;
	}

	if (leaves.count() == 2) {
		auto fst = leaves.first_set();
		auto snd = leaves.next_set(fst);
		auto result = m_cb.null_result();
		//#pragma omp critical
		result = m_cb.exit(m_cb.base_two_leaves(fst, snd));
		return result;
	}

	bitvector new_constraint_occ =
	        filter_constraints(leaves, constraint_occ, *m_constraints, c_occ_allocator());
	// base case: no constraints left
	if (new_constraint_occ.empty()) {
		auto result = m_cb.null_result();
		//#pragma omp critical
		result = m_cb.exit(m_cb.base_unconstrained(leaves));
		return result;
	}

	union_find sets = apply_constraints(leaves, new_constraint_occ, *m_constraints,
	                                    union_find_allocator());
	bipartitions bip_it(leaves, sets, leaf_allocator());

	return iterate(bip_it, new_constraint_occ);
}

template <typename Callback>
void tree_enumerator<Callback>::init_freelists(index leaf_count, index constraint_count) {
	index num_threads = static_cast<index>(std::max(1, omp_get_num_threads()));
	m_fl1.clear();
	m_fl2.clear();
	m_fl3.clear();
	m_fl1.resize(num_threads);
	m_fl2.resize(num_threads);
	m_fl3.resize(num_threads);
	m_fl1_allocsize = ranked_bitvector::alloc_size(leaf_count);
	m_fl2_allocsize = ranked_bitvector::alloc_size(constraint_count);
	m_fl3_allocsize = leaf_count;
}

template <typename Callback>
index tree_enumerator<Callback>::threadnum() const {
	return static_cast<index>(omp_get_thread_num());
}

template <typename Callback>
utils::stack_allocator<index> tree_enumerator<Callback>::leaf_allocator() {
	return {m_fl1[threadnum()], m_fl1_allocsize};
}

template <typename Callback>
utils::stack_allocator<index> tree_enumerator<Callback>::c_occ_allocator() {
	return {m_fl2[threadnum()], m_fl2_allocsize};
}

template <typename Callback>
utils::stack_allocator<index> tree_enumerator<Callback>::union_find_allocator() {
	return {m_fl3[threadnum()], m_fl3_allocsize};
}

template <typename Callback>
auto tree_enumerator<Callback>::iterate(bipartitions& bip_it, const bitvector& new_constraint_occ)
        -> result_type {
	if (bip_it.leaves().count() < 16 || bip_it.num_bip() < 4) {
		return iterate_sequential(bip_it, new_constraint_occ);
	} else {
		return iterate_parallel(bip_it, new_constraint_occ);
	}
}
template <typename Callback>
auto tree_enumerator<Callback>::iterate_parallel(bipartitions& bip_it,
                                                 const bitvector& new_constraint_occ)
        -> result_type {
	bool fast_return{};
	//#pragma omp critical
	fast_return = m_cb.fast_return(bip_it);
	if (fast_return) {
		auto result = m_cb.null_result();
		//#pragma omp critical
		result = m_cb.exit(m_cb.fast_return_value(bip_it));
		return result;
	}

	auto result = m_cb.null_result();
	//#pragma omp critical
	result = m_cb.begin_iteration(bip_it, new_constraint_occ, *m_constraints);
	// iterate over all possible bipartitions
#pragma omp taskgroup
	for (auto bip = bip_it.begin_bip();
	     bip < bip_it.end_bip() && m_cb.continue_iteration(result); ++bip) {
#pragma omp task shared(result, bip_it, new_constraint_occ)
		{
			m_cb.step_iteration(bip_it, bip);
			auto left_result = m_cb.null_result();
			auto right_result = m_cb.null_result();
			auto sets = bip_it.get_both_sets(bip, leaf_allocator());

#pragma omp task untied shared(left_result, sets, new_constraint_occ)
			{
				m_cb.left_subcall();
				left_result = run(sets.first, new_constraint_occ);
			}

#pragma omp task untied shared(right_result, sets, new_constraint_occ)
			{
				m_cb.right_subcall();
				right_result = run(sets.second, new_constraint_occ);
			}

#pragma omp taskwait
#pragma omp critical
			// accumulate result
			result = m_cb.accumulate(result, m_cb.combine(left_result, right_result));
		}
	}
	//#pragma omp critical
	m_cb.finish_iteration();

	//#pragma omp critical
	result = m_cb.exit(result);
	return result;
}

template <typename Callback>
auto tree_enumerator<Callback>::iterate_sequential(bipartitions& bip_it,
                                                   const bitvector& new_constraint_occ)
        -> result_type {
	if (m_cb.fast_return(bip_it)) {
		return m_cb.exit(m_cb.fast_return_value(bip_it));
	}

	auto result = m_cb.begin_iteration(bip_it, new_constraint_occ, *m_constraints);
	// iterate over all possible bipartitions
	for (auto bip = bip_it.begin_bip();
	     bip < bip_it.end_bip() && m_cb.continue_iteration(result); ++bip) {
		m_cb.step_iteration(bip_it, bip);
		auto set = bip_it.get_first_set(bip, leaf_allocator());
		m_cb.left_subcall();
		auto left_result = run(set, new_constraint_occ);
		bip_it.flip_set(set);
		m_cb.right_subcall();
		auto right_result = run(set, new_constraint_occ);
		// accumulate result
		result = m_cb.accumulate(result, m_cb.combine(left_result, right_result));
	}
	m_cb.finish_iteration();

	result = m_cb.exit(result);
	return result;
}

} // namespace terraces

#endif // SUPERTREE_ENUMERATOR_HPP
