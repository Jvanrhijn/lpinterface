#ifndef LPINTERFACE_TEST_UTIL_H
#define LPINTERFACE_TEST_UTIL_H

#include "lpinterface/data_objects.hpp"

namespace lpint {

template <class T>
inline Constraint<T> copy_constraint(const Constraint<T>& constr) {
  T lb = constr.lower_bound;
  T ub = constr.upper_bound;
  Row<T> row(constr.row.values(), constr.row.nonzero_indices());
  return Constraint<T>(std::move(row), lb, ub);
}

} // namespace lpint

#endif // LPINTERFACE_TEST_UTIL_H