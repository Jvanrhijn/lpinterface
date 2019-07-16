#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include "data_objects.hpp"

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
//std::vector<std::vector<T>> transpose(
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
SparseMatrix<T> build_sparse_matrix(std::array<std::array<T, N>, M> mat, SparseMatrixType sptype) {
  SparseMatrix<T> sp(sptype);

  if (sptype == SparseMatrixType::ColumnWise) {
    mat = transpose(mat);
  }

  std::vector<MatrixEntry<T>> entries;
  for (const auto& entry : mat) {
    std::vector<T> nonzeros;
    std::vector<T> v(entry.begin(), entry.end());
    std::copy_if(entry.begin(), entry.end(), std::back_inserter(nonzeros),
                 [](T x) { return x != T(); });
    entries.emplace_back(nonzeros, get_nonzero_indices(v));
  }
  if (sptype == SparseMatrixType::RowWise) {
    sp.add_rows(static_cast<std::vector<Row<T>>>(entries));
  } else {
    sp.add_columns(static_cast<std::vector<Column<T>>>(entries));
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseRowWise,
              (const std::array<std::array<double, N>, M> mat)) {
  SparseMatrix<double> sp(SparseMatrixType::RowWise);
  std::vector<Row<double>> rows;
  for (const auto& row : mat) {
    std::vector<double> nonzeros;
    std::vector<double> v(row.begin(), row.end());
    std::copy_if(row.begin(), row.end(), std::back_inserter(nonzeros),
                 [](double x) { return x != 0.0; });
    rows.emplace_back(nonzeros, get_nonzero_indices(v));
  }
  sp.add_rows(rows);
  //auto sp = build_sparse_matrix(mat, SparseMatrixType::RowWise);
  for (std::size_t i = 0; i < M; i++) {
    for (std::size_t j = 0; j < N; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseColumnWise,
              (const std::array<std::array<double, N>, M> mat)) {
  SparseMatrix<double> sp(SparseMatrixType::ColumnWise);
  std::vector<Column<double>> cols;

  auto mat_transpose = transpose(mat);

  for (const auto& col : mat_transpose) {
    std::vector<double> nonzeros;
    std::vector<double> v(col.begin(), col.end());
    std::copy_if(col.begin(), col.end(), std::back_inserter(nonzeros),
                 [](double x) { return x != 0.0; });
    cols.emplace_back(nonzeros, get_nonzero_indices(v));
  }

  sp.add_columns(cols);

  for (std::size_t i = 0; i < M; i++) {
    for (std::size_t j = 0; j < N; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}

//RC_GTEST_PROP(SparseMatrix, SparseMatrixIsIterable, 
//  (const std::array<std::array<double, N>, M> mat)) {
//  SparseMatrix<double> s
//}