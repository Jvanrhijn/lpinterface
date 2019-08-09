#ifndef LPINTERFACE_DATA_OBJECTS_H
#define LPINTERFACE_DATA_OBJECTS_H

#include <algorithm>
#include <cstddef>
#include <set>
#include <type_traits>
#include <vector>

// TODO: find a more elegant way to do this
#ifdef TESTING
#include <rapidcheck.h>
#endif

#include "common.hpp"
#include "errors.hpp"

namespace lpint {

/**
 * @brief Type of sparse matrices supported.
 */
enum class SparseMatrixType {
  //! CSR encoding.
  RowWise,
  //! CSC encoding.
  ColumnWise,
};

/**
 * @brief Enum representing orderings of values.
 */
enum class Ordering {
  //! Supported by:
  LT,
  //! Supported by:
  GT,
  //! Supported by: Gurobi.
  EQ,
  //! Supported by: Gurobi, SoPlex.
  LEQ,
  //! Supported by: Gurobi, SoPlex.
  GEQ,
};

/**
 * @brief Enum representing possible variable types for an LP.
 */
enum class VarType {
  //! Supported by: Gurobi.
  Binary,
  //! Supported by: Gurobi.
  Integer,
  //! Supported by: Gurobi, SoPlex.
  Real,
  //! Supported by: Gurobi.
  SemiReal,
  //! Supported by: Gurobi.
  SemiInteger,
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

 private:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

 public:
  using Index = int;
  using SizeType = std::size_t;

  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = int;
  using pointer = value_type*;
  using reference = value_type&;

  MatrixEntry(MatrixEntry<T>&&) = default;
  MatrixEntry(const MatrixEntry<T>&) = delete;

  MatrixEntry(const std::vector<T>& values, const std::vector<Index>& indices)
      : values_(values), nonzero_indices_(indices) {}
  virtual ~MatrixEntry() = default;

  MatrixEntry<T>& operator=(MatrixEntry<T>&&) = default;

  /**
   * @brief Indexing operator; can be used identically to a dense vector.
   * Will perform bounds checking ifndef NDEBUG.
   *
   * @param index Index of element to retrieve.
   * @return T Element at index.
   */
  T operator[](const SizeType index) const {
    auto index_in_data =
        std::find(nonzero_indices_.begin(), nonzero_indices_.end(), index);
    if (index_in_data == nonzero_indices_.end()) {
      return T();
    } else {
#if NDEBUG
      return values_[static_cast<SizeType>(index_in_data -
                                           nonzero_indices_.begin())];
#else
      return values_.at(
          static_cast<SizeType>(index_in_data - nonzero_indices_.begin()));
#endif
    }
  }

  //! Returns the smallest entry of this matrix element.
  T lower_bound() const {
    return std::min_element(values_.begin(), values_.end());
  }

  //! Returns the largest entry of this matrix element.
  T upper_bound() const {
    return std::max_element(values_.begin(), values_.end());
  }

  //! Return the number of nonzero entries in the matrix entry.
  typename std::vector<T>::size_type num_nonzero() const {
    return values_.size();
  }

  //! Get a const reference to the underlying value array.
  const std::vector<T>& values() const { return values_; }

  //! Get a reference to the underlying value array.
  std::vector<T>& values() { return values_; }

  //! Get a const reference to the underlying nonzero indices.
  const std::vector<Index>& nonzero_indices() const { return nonzero_indices_; }

  //! Get a reference to the underlying nonzero indices.
  std::vector<Index>& nonzero_indices() { return nonzero_indices_; }

  //! Obtain an iterator to the begin of the ests/CMakeFiles/unit_tests.dir/all] Error 2
  iterator begin() { return values_.begin(); }

  //! Obtain a const  iterator to the begin of the underlying value array.
  const_iterator begin() const { return values_.begin(); }

  //! Obtain an iterator to the end of the underlying value array.
  iterator end() { return values_.end(); }

  //! Obtain a const  iterator to the end of the underlying value array.
  const_iterator end() const { return values_.end(); }

 protected:
  std::vector<T> values_;
  std::vector<Index> nonzero_indices_;  // indices of nonzero entries
};

template <typename T>
class Column : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Column(Column<T>&&) = default;
  Column(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}
  Column(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
  Column() = default;
};

template <typename T>
class Row : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Row(Row<T>&&) = default;
  Row(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}

  Row(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
  Row() = default;
// TODO: find a more elegant way to do this
#if TESTING
  friend class rc::Arbitrary<Row<T>>;
#endif
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
 private:
  using iterator = typename std::vector<MatrixEntry<T>>::iterator;
  using const_iterator = typename std::vector<MatrixEntry<T>>::const_iterator;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::vector<T>;
  using difference_type = int;
  using pointer = value_type*;
  using reference = value_type&;

  //! Type of the nonzero matrix entry indices.
  using Index = typename MatrixEntry<T>::Index;
  //! Size type used to index into the sparse matrix.
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  SparseMatrix(SparseMatrix<T>&&) = default;
  SparseMatrix(const SparseMatrix<T>&) = delete;

  SparseMatrix() : type_(SparseMatrixType::RowWise) {}

  SparseMatrix(SparseMatrixType mtype) : type_(mtype) {}

  SparseMatrix(std::vector<Row<T>>&& rows) : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  SparseMatrix(std::vector<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  SparseMatrix(const std::initializer_list<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  SparseMatrix(const std::initializer_list<Row<T>>&& rows)
      : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  SparseMatrix<T>& operator=(SparseMatrix<T>&&) = default;

  //! Return an iterator to the begin of the underlying vector of entries
  iterator begin() { return entries_.begin(); }
  //! Return an iterator to the end of the underlying vector of entries
  iterator end() { return entries_.end(); }

  //! Return a const iterator to the begin of the underlying vector of entries
  const_iterator begin() const { return entries_.begin(); }
  //! Return a const iterator to the end of the underlying vector of entries
  const_iterator end() const { return entries_.end(); }

  /**
   * @brief Return the number of matrix entries.
   * If type() == SparseMatrixType::RowWise, returns the number of rows.
   * If type() == SparseMatrixType::ColumnWise, returns the number of columns.
   *
   * @return std::size_t Number of matrix entries.
   */
  typename std::vector<MatrixEntry<T>>::size_type num_entries() const {
    return entries_.size();
  }

  const std::vector<MatrixEntry<T>>& entries() const { return entries_; }

  /**
   * @brief Add columns to the sparse matrix.
   *
   * @param columns Vector of columns to add. Ownership of the data is
   * transferred to the matrix.
   */
  void add_columns(std::vector<Column<T>>&& columns) {
    if (type_ != SparseMatrixType::ColumnWise) {
      throw MatrixTypeException();
    } else {
      for (auto& entry : columns) {
        check_entry_valid(entry);
        entries_.emplace_back(std::move(entry));
      }
    }
  }

  /**
   * @brief Add rows to the sparse matrix.
   *
   * @param rows Vector of rows to add. Ownership of the data is transferred to
   * the matrix.
   */
  void add_rows(std::vector<Row<T>>&& rows) {
    if (type_ != SparseMatrixType::RowWise) {
      throw MatrixTypeException();
    } else {
      for (auto& entry : rows) {
        // entries are invalid if there are two duplicate
        // nonzero indices present
        check_entry_valid(entry);
        entries_.emplace_back(std::move(entry));
      }
    }
  }

  /**
   * @brief Matrix indexing operator. Performs bounds checking ifndef NDEBUG.
   *
   * @param i Row index of element to access.
   * @param j Column index of element to access.
   * @return T Element at matrix position A_{ij}.
   */
  T operator()(const SizeType i, const SizeType j) const {
#ifdef NDEBUG
    if (type_ == SparseMatrixType::ColumnWise) {
      return entries_[j][i];
    } else {
      return entries_[i][j];
    }
#else
    if (type_ == SparseMatrixType::ColumnWise) {
      return entries_.at(j)[i];
    } else {
      return entries_.at(i)[j];
    }
#endif
  }

  SparseMatrixType type() const { return type_; }

 private:
  void add_entries(const std::vector<MatrixEntry<T>>& entries) {
    for (const auto& entry : entries) {
      check_entry_valid(entry);
      entries_.emplace_back(entry);
    }
  }

  void check_entry_valid(const MatrixEntry<T>& entry) {
    // matrix entries are invalid if there are
    // duplicate nonzero indices present
    const auto& nonzero_indices = entry.nonzero_indices();
    std::set<Index> index_set(nonzero_indices.begin(), nonzero_indices.end());
    if (nonzero_indices.size() != index_set.size()) {
      throw InvalidMatrixEntryException();
    }
  }

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
  //! Ordering type of this constraint, see Ordering for possible variants.
  Ordering ordering;
  //! Value of right-hand-side vector of constraints.
  T value;
};

/**
 * @brief Struct representing the objective vector.
 * A linear program has the canonical form
 * \f[
 *    \max c^T x.
 * \f]
 * This structure represents the vector \f$c\f$.
 * @tparam T Type of elements in the objective vector.
 */
template <typename T>
struct Objective {
  //! Values of elements in the objective vector.
  std::vector<T> values;
  //! Variable type the objective assigns to each
  //! variable the linear program optimizes.
  std::vector<VarType> variable_types;
};

/**
 * @brief Struct representing the solution of a linear program.
 *
 * @tparam T Type of elements in the solution vector \f$x\f$.
 */
template <typename T>
struct Solution {
  //! Values in the primal solution vector.
  std::vector<T> primal;
  //! Values in the dual solution vector.
  std::vector<T> dual;
  //! Value of the objective \f$c^T x\f$.
  T objective_value;
};

inline std::ostream& operator<<(std::ostream& os, const Ordering& ord) {
  switch (ord) {
    case Ordering::EQ:
      os << "EQ";
      break;
    case Ordering::GEQ:
      os << "GEQ";
      break;
    case Ordering::GT:
      os << "GT";
      break;
    case Ordering::LEQ:
      os << "LEQ";
      break;
    case Ordering::LT:
      os << "LT";
      break;
    default:
      throw UnsupportedConstraintException();
  }
  return os;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os,
                                const Constraint<T>& constraint) {
  os << constraint.ordering << " " << constraint.value;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const VarType& vtype) {
  switch (vtype) {
    case VarType::Binary:
      os << "Binary";
      break;
    case VarType::Integer:
      os << "Integer";
      break;
    case VarType::Real:
      os << "Real";
      break;
    case VarType::SemiInteger:
      os << "Semi-integer";
      break;
    case VarType::SemiReal:
      os << "Semi-real";
      break;
    default:
      throw UnsupportedVariableTypeException();
  }
  return os;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Objective<T>& obj) {
  if (obj.values.size() == 0) {
    os << "Objective {}";
    return os;
  }
  os << "Objective {";
  std::size_t n = obj.values.size();
  for (std::size_t i = 0; i < n; i++) {
    os << obj.variable_types[i] << " " << obj.values[i] << ", ";
  }
  os << "\b\b}";
  return os;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const MatrixEntry<T>& row) {
  if (row.num_nonzero() == 0) {
    os << "Entry {[] []}";
    return os;
  }
  os << "Entry {[";
  for (const auto& val : row.values()) {
    os << val << " ";
  }
  os << "\b] [";
  for (const auto& val : row.nonzero_indices()) {
    os << val << " ";
  }
  os << "\b]}";
  return os;
}

}  // namespace lpint

#endif  // LPINTERFACE_DATA_OBJECTS_H