#ifndef LPINTERFACE_DATA_OBJECTS_H
#define LPINTERFACE_DATA_OBJECTS_H

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "common.hpp"
#include "errors.hpp"

namespace lpint {

enum class SparseMatrixType {
  RowWise,
  ColumnWise,
};

enum class Ordering {
  LT,
  GT,
  EQ,
  LEQ,
  GEQ,
};

// matrix entry is templated over T, with T restricted to
// arithmetic types i.e. numbers
/**
 * @brief Matrix entry type, for use in sparse matrix.
 * The matrix entry base class is specialized as Row and Column,
 * for the two sparse matrix representations. The class
 * thus represents a row or column vector with mostly zero
 * entries. Its operator[] is overloaded to provide
 * access as if it is a dense vector. Access using operator[]
 * is O(n) in time, with n the number of nonzero entries.
 */
template <typename T>
class MatrixEntry {
  static_assert(std::is_arithmetic<T>::value,
                "MatrixEntry<T> requires T to be arithmetic");

 public:
  MatrixEntry(const std::vector<T>& values,
              const std::vector<std::size_t>& indices)
      : values_(values), nonzero_indices_(indices) {}

  virtual ~MatrixEntry() = default;

  T operator[](const std::size_t index) const {
    auto index_in_data =
        std::find(nonzero_indices_.begin(), nonzero_indices_.end(), index);
    if (index_in_data == nonzero_indices_.end()) {
      return T();
    } else {
      // TODO replace with safe indexing in debug case
      return values_[static_cast<std::size_t>(index_in_data -
                                              nonzero_indices_.begin())];
    }
  }

  T lower_bound() const {
    return std::min_element(values_.begin(), values_.end());
  }
  T upper_bound() const {
    return std::max_element(values_.begin(), values_.end());
  }
  std::size_t num_nonzero() const { return values_.size(); }
  
  const std::vector<T>& values() const { return values_; }
  const std::vector<std::size_t>& nonzero_indices() const { return nonzero_indices_; }

 private:
  std::vector<T> values_;
  std::vector<std::size_t> nonzero_indices_;  // indices of nonzero entries
};

template <typename T>
class Column : public MatrixEntry<T> {
 public:
  Column(const std::vector<T>& values, const std::vector<std::size_t>& indices)
      : MatrixEntry<T>(values, indices) {}
  Column(const MatrixEntry<T> m) : MatrixEntry<T>(m) {}
  Column() = default;
};

template <typename T>
class Row : public MatrixEntry<T> {
 public:
  Row(const std::vector<T>& values, const std::vector<std::size_t>& indices)
      : MatrixEntry<T>(values, indices) {}

  Row(const MatrixEntry<T> m) : MatrixEntry<T>(m) {}
  Row() = default;
};

/**
 * @brief Represents a sparse matrix.
 * This class represents either a row or column oriented
 * sparse matrix. In order to access matrix elements, use
 * operator(), i.e. `double x = matrix(3, 5);`. Access
 * is O(n) in time.
 */
template <typename T>
class SparseMatrix {
 public:
  SparseMatrix(SparseMatrixType mtype) : type_(mtype) {}

  expected<void, LpError> add_columns(const std::vector<Column<T>>& columns) {
    if (type_ != SparseMatrixType::ColumnWise) {
      return unexpected<LpError>(LpError::MatrixTypeError);
    } else {
      for (const auto& entry : columns) {
        entries_.emplace_back(entry);
      }
      return expected<void, LpError>();
    }
  }

  expected<void, LpError> add_rows(const std::vector<Row<T>>& rows) {
    if (type_ != SparseMatrixType::RowWise) {
      return unexpected<LpError>(LpError::MatrixTypeError);
    } else {
      for (const auto& entry : rows) {
        entries_.emplace_back(entry);
      }
      return expected<void, LpError>();
    }
  }

  T operator()(const std::size_t i, const std::size_t j) const {
    if (type_ == SparseMatrixType::ColumnWise) {
      return entries_[j][i];
    } else {
      return entries_[i][j];
    }
  }

  SparseMatrixType type() const { return type_; }

 private:
  SparseMatrixType type_;
  std::vector<std::size_t> begin_indices_;
  std::vector<MatrixEntry<T>> entries_;
};

/**
 * @brief Struct to represent right-hand side of LP constraints.
 * In linear programming, we have constraints of the form
 \f[
   Ax <\mathrm{cmp}> b,
 \f]
 where \f$<cmp>\f$ represents an elementwise comparison operator,
 such as \f$\leq\f$. This struct represents one element of the right-hand side
 of such a constraint, together with the elementwise comparison.
 */
template <typename T>
struct Constraint {
  static_assert(std::is_arithmetic<T>::value,
                "T must be arithmetic in order to be ordered");
  Ordering ordering;
  T value;
};

class Objective {};

}  // namespace lpint

#endif  // LPINTERFACE_DATA_OBJECTS_H