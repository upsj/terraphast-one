#include <terraces/bitmatrix.hpp>

#include <cassert>

namespace terraces {

bitmatrix::bitmatrix(index_t rows, index_t cols) : m_rows{rows}, m_cols{cols}, m_vec(rows * cols) {}

index_t bitmatrix::rows() const { return m_rows; }
index_t bitmatrix::cols() const { return m_cols; }

bool bitmatrix::get(index_t row, index_t col) const {
	assert(row < m_rows && col < m_cols);
	return m_vec[row * m_cols + col];
}

void bitmatrix::set(index_t row, index_t col, bool val) {
	assert(row < m_rows && col < m_cols);
	m_vec[row * m_cols + col] = val;
}

void bitmatrix::row_or(index_t in1, index_t in2, index_t out) {
	for (index_t i = 0; i < cols(); ++i) {
		set(out, i, get(in1, i) || get(in2, i));
	}
}

bitmatrix bitmatrix::get_cols(const std::vector<std::size_t>& cols) const {
	assert(cols.size() <= this->cols());
	auto ret = bitmatrix{rows(), cols.size()};
	for (auto i = std::size_t{}; i < rows(); ++i) {
		for (auto j = std::size_t{}; j < cols.size(); ++j) {
			ret.set(i, j, get(i, cols[j]));
		}
	}
	return ret;
}

bool bitmatrix::operator==(const bitmatrix& other) const {
	return other.rows() == rows() && other.cols() == cols() && other.m_vec == m_vec;
}

bool bitmatrix::operator!=(const bitmatrix& other) const { return !(*this == other); }

} // namespace terraces
