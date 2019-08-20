#ifndef LPINTERFACE_DATA_OBJECTS_H
#define LPINTERFACE_DATA_OBJECTS_H

#include <algorithm>
#include <cstddef>
#include <set>
#include <type_traits>
#include <vector>

#include "common.hpp"
#include "errors.hpp"

namespace lpint {

/// Enum representing orderings of values.
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

/// Enum representing possible variable types for an LP.
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

  MatrixEntry() = default;
  MatrixEntry(MatrixEntry<T>&&) = default;
  MatrixEntry(const MatrixEntry<T>&) = delete;
  MatrixEntry<T>& operator=(const MatrixEntry<T>&) = delete;
  MatrixEntry<T>& operator=(MatrixEntry<T>&&) = default;

  MatrixEntry(const std::vector<T>& values, const std::vector<Index>& indices)
      : values_(values), nonzero_indices_(indices) {
    if (values_.size() != nonzero_indices_.size()) {
      throw MismatchedDimensionsException();
    }
    check_entry_valid();
  }
  virtual ~MatrixEntry() = default;

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

  //! Obtain an iterator to the begin of the ests/CMakeFiles/unit_tests.dir/all]
  //! Error 2
  iterator begin() { return values_.begin(); }

  //! Obtain a const  iterator to the begin of the underlying value array.
  const_iterator begin() const { return values_.begin(); }

  //! Obtain an iterator to the end of the underlying value array.
  iterator end() { return values_.end(); }

  //! Obtain a const  iterator to the end of the underlying value array.
  const_iterator end() const { return values_.end(); }

 private:
  std::vector<T> values_;
  std::vector<Index> nonzero_indices_;  // indices of nonzero entries

  void check_entry_valid() {
    // matrix entries are invalid if there are
    // duplicate nonzero indices present
    std::set<Index> index_set(nonzero_indices_.begin(), nonzero_indices_.end());
    if (nonzero_indices_.size() != index_set.size()) {
      throw InvalidMatrixEntryException();
    }
  }
};

template <typename T>
class Column : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Column() = default;
  Column(Column<T>&&) = default;
  Column(const Column<T>&) = delete;

  Column<T>& operator=(Column<T>&&) = default;
  Column<T>& operator=(const Column<T>&) = delete;

  ~Column() = default;

  Column(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}
  explicit Column(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
};

template <typename T>
class Row : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Row() = default;

  Row(const Row<T>&) = delete;
  Row(Row<T>&&) = default;
  Row<T>& operator=(Row<T>&&) = default;
  Row<T>& operator=(const Row<T>&) = delete;
  ~Row() = default;

  Row(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}
  explicit Row(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
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

  Constraint() : row(), lower_bound(), upper_bound() {}
  Constraint(Row<T>&& r, T lb, T ub)
      : row(std::move(r)), lower_bound(lb), upper_bound(ub) {}

  Row<T> row;
  //! Ordering type of this constraint, see Ordering for possible variants.
  //Ordering ordering;
  //! Lower bound of constraint equation.
  T lower_bound;
  //! Upper bound of constraint equation.
  T upper_bound;
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
  Objective() = default;
  // We can leave variable_types empty, since
  // Gurobi assumes continuous variables by default.
  // This might be different in other solvers, so
  // we'll have to carefully check when adding support.
  explicit Objective(std::vector<T>&& vals) : values(std::move(vals)) {}
  Objective(std::vector<T>&& vals, std::vector<VarType>&& var_types)
      : values(std::move(vals)), variable_types(std::move(var_types)) {
    if (values.size() != variable_types.size()) {
      throw MismatchedDimensionsException();
    }
  }
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

// LCOV_EXCL_START
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
  os << constraint.lower_bound << " <= " << constraint.row << " <= " << constraint.upper_bound;
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
// LCOV_EXCL_STOP

}  // namespace lpint

#endif  // LPINTERFACE_DATA_OBJECTS_H