#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include "data_objects.hpp"

#include "data_objects.hpp"

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

template <typename T, size_t N>
std::vector<std::vector<T>> transpose(
    const std::array<std::array<T, N>, N>& v) {
  std::size_t m = v.size();  // number of rows
  if (m == 0) {
    return std::vector<std::vector<T>>(0);
  }

  std::size_t n = v[0].size();
  std::vector<std::vector<T>> out(n, std::vector<T>(m));

  for (std::size_t i = 0; i < m; i++) {
    for (std::size_t j = 0; j < m; j++) {
      out[i][j] = v[j][i];
    }
  }
  return out;
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseRowWise,
              (const std::array<std::array<double, 10>, 10> mat)) {
  constexpr std::size_t m = 10;
  constexpr std::size_t n = 10;

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
  for (std::size_t i = 0; i < m; i++) {
    for (std::size_t j = 0; j < n; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseColumnWise,
              (const std::array<std::array<double, 10>, 10> mat)) {
  constexpr std::size_t m = 10;
  constexpr std::size_t n = 10;

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

  for (std::size_t i = 0; i < m; i++) {
    for (std::size_t j = 0; j < n; j++) {
      RC_ASSERT(sp(i, j) == mat[i][j]);
    }
  }
}