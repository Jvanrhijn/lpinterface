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

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDense,
              (const int n, const std::array<std::array<double, 10>, 10> mat)) {
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
  RC_ASSERT(false);
}