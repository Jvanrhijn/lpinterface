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
#include "sparse_matrix.hpp"

namespace lpint {

/// Enum representing orderings of values.
enum class Ordering {
  LT, /// Supported by:
  GT, /// Supported by:
  EQ, /// Supported by: Gurobi.
  LEQ, /// Supported by: Gurobi, SoPlex.
  GEQ, /// Supported by: Gurobi, SoPlex.
};

/// Enum representing possible variable types for an LP.
enum class VarType {
  Binary, /// Supported by: Gurobi.
  Integer, /// Supported by: Gurobi.
  Real, /// Supported by: Gurobi, SoPlex.
  SemiReal, /// Supported by: Gurobi.
  SemiInteger, /// Supported by: Gurobi.
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

  Constraint() = default;
  Constraint(Row<T>&& r, Ordering ord, T val)
      : row(std::move(r)), ordering(ord), value(val) {}

  Row<T> row;
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
  Objective() = default;
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
  os << constraint.row << " " << constraint.ordering << " " << constraint.value;
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