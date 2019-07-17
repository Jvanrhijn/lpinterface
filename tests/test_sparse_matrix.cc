#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include "lpinterface.hpp"

using namespace lpint;

constexpr std::size_t M = 10;
constexpr std::size_t N = 10;

template <typename T>
std::vector<std::size_t> get_nonzero_indices(const std::vector<T>& v) {
  std::vector<std::size_t> nz;
  for (std::size_t i = 0; i < v.size(); i++) {
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
    sp.add_rows(rows);
  } else {
    sp.add_columns(cols);
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
              (const std::array<std::array<double, N>, M> mat)) {
  auto sp = build_sparse_matrix_row(mat);
  for (std::size_t i = 0; i < M; i++) {
    for (std::size_t j = 0; j < N; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseColumnWise,
              (const std::array<std::array<double, N>, M> mat)) {
  auto sp = build_sparse_matrix_col(mat);
  for (std::size_t i = 0; i < M; i++) {
    for (std::size_t j = 0; j < N; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}

TEST(SparseMatrix, ErrorIfDuplicateNonzeroIndices) {
  SparseMatrix<double> sp(SparseMatrixType::RowWise);
  auto err = sp.add_rows({Row<double>({1, 2, 3}, {0, 0, 1})});
  ASSERT_EQ(err.error(), LpError::InvalidMatrixEntryError);
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIsIterable,
              (const std::array<std::array<double, N>, M> mat)) {
  auto sp = build_sparse_matrix_row(mat);
  std::size_t i = 0;
  for (const auto& row : sp) {
    std::vector<double> vec(mat[i].begin(), mat[i].end());
    for (std::size_t j : row.nonzero_indices()) {
      RC_ASSERT(row[j] == vec[j]);
    }
    i++;
  }

  auto spc = build_sparse_matrix_col(mat);
  auto matc = transpose(mat);
  i = 0;
  for (const auto& col : spc) {
    std::vector<double> vec(matc[i].begin(), matc[i].end());
    for (std::size_t j : col.nonzero_indices()) {
      RC_ASSERT(col[j] == vec[j]);
    }
    i++;
  }
}
