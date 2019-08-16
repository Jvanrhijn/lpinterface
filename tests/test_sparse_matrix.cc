#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include "generators.hpp"
#include "lpinterface.hpp"

using namespace lpint;

using Index = typename MatrixEntry<double>::Index;
using SizeType = typename MatrixEntry<double>::SizeType;

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseRowWise, ()) {
  auto nrows = *rc::gen::nonZero<uint8_t>();
  auto ncols = *rc::gen::inRange<uint8_t>(2, std::numeric_limits<uint8_t>::max());
  SparseMatrix<double> sp(SparseMatrixType::RowWise);
  sp.add_rows(*rc::gen::container<std::vector<Row<double>>>(
      nrows, rc::genRow(ncols, rc::gen::nonZero<double>())));
  for (SizeType i = 0; i < nrows; ++i) {
    for (SizeType j = 0; j < ncols; ++j) {
      const auto nz = sp.entries()[i].nonzero_indices();
      if (std::find(nz.begin(), nz.end(), j) != nz.end()) {
        RC_ASSERT(sp(i, j) != 0);
      } else {
        RC_ASSERT(sp(i, j) == 0);
      }
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIndexesLikeDenseColumnWise, ()) {
  auto nrows = *rc::gen::nonZero<uint8_t>();
  auto ncols = *rc::gen::inRange<uint8_t>(2, std::numeric_limits<uint8_t>::max());
  SparseMatrix<double> sp(SparseMatrixType::ColumnWise);
  auto cols = *rc::gen::container<std::vector<Column<double>>>(
      ncols, rc::genRow(ncols, rc::gen::nonZero<double>()));
  sp.add_columns(std::move(cols));
  for (SizeType i = 0; i < nrows; ++i) {
    for (SizeType j = 0; j < ncols; ++j) {
      const auto nz = sp.entries()[j].nonzero_indices();
      if (std::find(nz.begin(), nz.end(), i) != nz.end()) {
        RC_ASSERT(sp(i, j) != 0);
      } else {
        RC_ASSERT(sp(i, j) == 0);
      }
    }
  }
}

RC_GTEST_PROP(SparseMatrix, SparseMatrixIsIterable, ()) {
  auto nrows = *rc::gen::nonZero<uint8_t>();
  auto ncols = *rc::gen::inRange<uint8_t>(2, std::numeric_limits<uint8_t>::max());
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