#ifndef LPINTERFACE_TEST_UTIL_H
#define LPINTERFACE_TEST_UTIL_H

#include <type_traits>
#include <string>
#include <sstream>
#include <rapidcheck.h>

#include "lpinterface/data_objects.hpp"

namespace lpint {

template <class T>
inline Constraint<T> copy_constraint(const Constraint<T>& constr) {
  T lb = constr.lower_bound;
  T ub = constr.upper_bound;
  Row<T> row(constr.row.values(), constr.row.nonzero_indices());
  return Constraint<T>(std::move(row), lb, ub);
}

template <class MetaFunction, class... Ts>
inline void for_each_type() {
  auto dummy = {(MetaFunction::template exec<Ts>(), 0)...};
  static_cast<void>(dummy);
}

template <class T, class Testable>
void templated_prop(const std::string& description, Testable testable) {
  std::cerr << description + " (" + typeid(T).name() + ")" << std::endl;
  rc::detail::checkGTest(testable);
}

} // namespace lpint

#endif // LPINTERFACE_TEST_UTIL_H