#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include "generators.hpp"
#include "lpinterface.hpp"

using namespace lpint;

using Index = typename MatrixEntry<double>::Index;
using SizeType = typename MatrixEntry<double>::SizeType;

constexpr Index M = 10;
constexpr Index N = 10;

template <typename T>
std::vector<Index> get_nonzero_indices(const std::vector<T>& v) {
  std::vector<Index> nz;
  for (SizeType i = 0; i < v.size(); i++) {
    if (v[i] != T()) {
      nz.push_back(i);
    }
  }
  return nz;
}

template <typename T, size_t N, size_t M>
// std::vector<std::vector<T>> transpose(
std::array<std::array<T, M>, N> transpose(
    const std::array<std::array<T, N>, M>& v) {
  std::array<std::array<T, M>, N> out;
  for (std::size_t i = 0; i < N; i++) {
    for (std::size_t j = 0; j < M; j++) {
      out[i][j] = v[j][i];
    }
  }
  return out;
}

template <typename T, size_t N, size_t M>
SparseMatrix<T> build_sparse_matrix_base(
    const std::array<std::array<T, N>, M>& mat, SparseMatrixType sptype) {
  SparseMatrix<T> sp(sptype);
  std::vector<Row<T>> rows;
  std::vector<Column<T>> cols;
  for (const auto& entry : mat) {
    std::vector<T> nonzeros;
    std::vector<T> v(entry.begin(), entry.end());
    std::copy_if(entry.begin(), entry.end(), std::back_inserter(nonzeros),
                 [](T x) { return x != T(); });
    if (sptype == SparseMatrixType::RowWise) {
      rows.emplace_back(nonzeros, get_nonzero_indices(v));
    } else {
      cols.emplace_back(nonzeros, get_nonzero_indices(v));
    }
  }
  if (sptype == SparseMatrixType::RowWise) {
    sp.add_rows(std::move(rows));
  } else {
    sp.add_columns(std::move(cols));
  }
  return sp;
}

template <typename T, size_t N, size_t M>
SparseMatrix<T> build_sparse_matrix_col(std::array<std::array<T, N>, M> mat) {
  auto mat_ = transpose(mat);
  return build_sparse_matrix_base(mat_, SparseMatrixType::ColumnWise);
}

template <typename T, size_t N, size_t M>
SparseMatrix<T> build_sparse_matrix_row(std::array<std::array<T, N>, M> mat) {
  return build_sparse_matrix_base(mat, SparseMatrixType::RowWise);
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseRowWise,
              (const uint8_t nrows, const uint8_t ncols)) {
  SparseMatrix<double> sp(SparseMatrixType::RowWise);
  sp.add_rows(*rc::gen::container<std::vector<Row<double>>>(
      nrows, rc::genRow(ncols, rc::gen::nonZero<double>())));
  for (SizeType i = 0; i < nrows; i++) {
    for (SizeType j = 0; j < ncols; j++) {
      const auto nz = sp.entries()[i].nonzero_indices();
      if (std::find(nz.begin(), nz.end(), j) != nz.end()) {
        RC_ASSERT(sp(i, j) != 0);
      } else {
        RC_ASSERT(sp(i, j) == 0);
      }
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseColumnWise,
              (const uint8_t nrows, const uint8_t ncols)) {
  SparseMatrix<double> sp(SparseMatrixType::ColumnWise);
  auto cols = *rc::gen::container<std::vector<Column<double>>>(
      ncols, rc::genRow(ncols, rc::gen::nonZero<double>()));
  sp.add_columns(std::move(cols));
  for (SizeType i = 0; i < nrows; i++) {
    for (SizeType j = 0; j < ncols; j++) {
      const auto nz = sp.entries()[j].nonzero_indices();
      if (std::find(nz.begin(), nz.end(), i) != nz.end()) {
        RC_ASSERT(sp(i, j) != 0);
      } else {
        RC_ASSERT(sp(i, j) == 0);
      }
    }
  }
}

RC_GTEST_PROP(SparseMatrix, ErrorIfDuplicateNonzeroIndices, ()) {
  SparseMatrix<double> sp(SparseMatrixType::RowWise);

  const auto values = *rc::gen::container<std::vector<double>>(
      *rc::gen::inRange(uint8_t(2), std::numeric_limits<uint8_t>::max()),
      rc::gen::arbitrary<double>());
  auto indices = *rc::gen::uniqueCount<std::vector<Index>>(
      values.size(), rc::gen::inRange(0ul, values.size()));

  const auto from = *rc::gen::inRange(0ul, values.size() / 2);
  const auto to = *rc::gen::inRange(from + 1, values.size());
  indices[to] = indices[from];  // make a duplicate index
  try {
    std::vector<Row<double>> rows;
    rows.emplace_back(values, indices);
    sp.add_rows(std::move(rows));
    RC_ASSERT(false);
  } catch (const InvalidMatrixEntryException& e) {
    RC_ASSERT(true);
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIsIterable,
              (const uint8_t nrows, const uint8_t ncols)) {
  SparseMatrix<double> sp(SparseMatrixType::RowWise);
  sp.add_rows(*rc::gen::container<std::vector<Row<double>>>(
      nrows, rc::genRow(ncols, rc::gen::nonZero<double>())));
  for (const auto& row : sp) {
    for (const auto& val : row) {
      RC_ASSERT(val != 0);
    }
  }

  SparseMatrix<double> spc(SparseMatrixType::ColumnWise);
  spc.add_columns(*rc::gen::container<std::vector<Column<double>>>(
      nrows, rc::genColumn(ncols, rc::gen::nonZero<double>())));
  for (const auto& col : spc) {
    for (const auto& val : col) {
      RC_ASSERT(val != 0);
    }
  }
}