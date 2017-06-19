#ifndef FAST_SET_H
#define FAST_SET_H

#include <cassert>
#include <terraces/bitvector.hpp>

namespace terraces {

class fast_index_set_iterator;

class fast_index_set {
	using iterator = fast_index_set_iterator;
	friend class fast_index_set_iterator;

private:
	efficient::bitvector m_vector;

public:
	using value_type = efficient::bitvector::value_type;

	fast_index_set(index max_size) : m_vector{max_size} {}

	bool contains(index i) const { return m_vector.get(i); }
	index rank(index i) const {
		assert(contains(i));
		return m_vector.rank(i);
	}
	index select(index i) const;

	index max_size() const { return m_vector.size(); }
	index size() const { return m_vector.count(); }

	iterator begin() const;
	iterator end() const;

	void insert(index i) { m_vector.set(i); }
	void remove(index i) { m_vector.clr(i); }
	void toggle(index i) { m_vector.flip(i); }
	void symm_difference(const fast_index_set& other) { m_vector.bitwise_xor(other.m_vector); }
	void complement() { m_vector.invert(); }
	void clear() { m_vector.blank(); }
	void finalize_edit() { m_vector.update_ranks(); }
};

class fast_index_set_iterator {
public:
	using value_type = fast_index_set::value_type;

private:
	const fast_index_set& m_set;
	index m_index;

public:
	fast_index_set_iterator(const fast_index_set& set, index i) : m_set{set}, m_index{i} {}
	fast_index_set_iterator& operator++() {
		m_index = m_set.m_vector.next(m_index);
		return *this;
	}
	bool operator==(const fast_index_set_iterator& other) const {
		return m_index == other.m_index;
	}
	bool operator!=(const fast_index_set_iterator& other) const { return !(*this == other); }
	const index& operator*() const { return m_index; }
};

inline auto fast_index_set::begin() const -> iterator { return {*this, m_vector.begin()}; }

inline auto fast_index_set::end() const -> iterator { return {*this, m_vector.end()}; }

inline index fast_index_set::select(index i) const {
	assert(i < size());
	auto it = begin();
	for (index j = 0; j < i; ++j) {
		++it;
	}
	return *it;
}

} // namespace terraces

#endif // FAST_SET_H
