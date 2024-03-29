#ifndef LPINTERFACE_GENERATORS_H
#define LPINTERFACE_GENERATORS_H

#include <iostream>
#include <tuple>
#include <algorithm>

#include "rapidcheck.h"

#include "lpinterface.hpp"

namespace rc {

template <typename T>
inline Gen<std::vector<T>> genUniqueVector(std::size_t size, Gen<T> gen) {
  return gen::suchThat(gen::unique<std::vector<T>>(gen),
                       [&](std::vector<T> v) { return v.size() == size; });
}

template <typename T>
struct Arbitrary<lpint::Objective<T>> {
  static Gen<lpint::Objective<T>> arbitrary() {
    return gen::build<lpint::Objective<T>>(
        gen::set(&lpint::Objective<T>::values));
  }
};

template <typename T>
inline Gen<lpint::Constraint<T>> genConstraint(
    Gen<lpint::Row<T>> rowgen, Gen<T> vgen) {
  auto bound1 = *vgen;
  auto bound2 = *rc::gen::distinctFrom(vgen, bound1);
  auto ub = std::max(bound1, bound2);
  auto lb = std::min(bound1, bound2);
  return gen::build<lpint::Constraint<T>>(
      gen::set(&lpint::Constraint<T>::row, rowgen),
      gen::set(&lpint::Constraint<T>::lower_bound, rc::gen::just(lb)),
      gen::set(&lpint::Constraint<T>::upper_bound, rc::gen::just(ub)));
}

template <typename T>
inline Gen<lpint::Objective<T>> genSizedObjective(std::size_t size,
                                                  Gen<T> valgen) {
  return gen::build<lpint::Objective<T>>(
      gen::set(&lpint::Objective<T>::values,
               gen::container<std::vector<T>>(size, valgen)));
}

// if fixed == true, count is the number of nonzero elements in the row.
// if fixed == false, count is the maximum number of nonzero elements in the
// row.
template <typename T>
inline Gen<lpint::Row<T>> genRow(const std::size_t count, Gen<T> valgen,
                                 bool fixed = false) {
  using namespace lpint;
  std::size_t ncols = fixed ? count : *rc::gen::inRange(1ul, count);
  return gen::construct<Row<T>>(
      rc::gen::container<std::vector<T>>(ncols, std::move(valgen)),
      rc::gen::unique<std::vector<typename Row<T>::Index>>(
          ncols, rc::gen::inRange(0ul, count)));
}

template <typename T>
inline Gen<lpint::Row<T>> genRow(Gen<T> valgen) {
  using namespace lpint;
  const auto count = *rc::gen::nonZero<std::size_t>();
  return gen::construct<Row<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::unique<std::vector<typename Row<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
struct Arbitrary<lpint::Row<T>> {
  static Gen<lpint::Row<T>> arbitrary() {
    using namespace lpint;
    return genRow(rc::gen::arbitrary<T>());
  }
};

// TODO: fix s.t. lower_bound <= upper_bound
template <typename T>
struct Arbitrary<lpint::Constraint<T>> {
  static Gen<lpint::Constraint<T>> arbitrary() {
    return gen::build<lpint::Constraint<T>>(
        gen::set(&lpint::Constraint<T>::lower_bound),
        gen::set(&lpint::Constraint<T>::row),
        gen::set(&lpint::Constraint<T>::upper_bound));
  }
};

template <typename T>
inline Gen<lpint::Column<T>> genColumn(const std::size_t count, Gen<T> valgen) {
  using namespace lpint;
  return gen::construct<Column<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::unique<std::vector<typename Column<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
inline Gen<lpint::Column<T>> genColumn(Gen<T> valgen) {
  using namespace lpint;
  const auto count = *rc::gen::arbitrary<std::size_t>();
  return gen::construct<Column<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::unique<std::vector<typename Column<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
struct Arbitrary<lpint::Column<T>> {
  static Gen<lpint::Column<T>> arbitrary() {
    using namespace lpint;
    return genColumn(rc::gen::arbitrary<T>());
  }
};

template <>
struct Arbitrary<lpint::OptimizationType> {
  static Gen<lpint::OptimizationType> arbitrary() {
    return gen::element(lpint::OptimizationType::Maximize,
                        lpint::OptimizationType::Minimize);
  }
};

template <>
struct Arbitrary<lpint::Variable> {
  static Gen<lpint::Variable> arbitrary() {
    using namespace lpint;
    return gen::construct<Variable>(
      gen::negative<double>(),
      gen::nonNegative<double>()
    );
  }
};

template <class Solver>
inline Solver genLinearProgramSolver(
    const std::size_t max_nrows, const std::size_t max_ncols) {
  using namespace lpint;

  const std::size_t nrows =
      *rc::gen::inRange<std::size_t>(1, max_nrows).as("Rows in LP");
  const std::size_t ncols =
      *rc::gen::inRange<std::size_t>(1, max_ncols).as("Columns in LP");

  auto constraints = *rc::gen::container<std::vector<Constraint<double>>>(
    nrows, 
    rc::genConstraint(
      genRow(ncols, rc::gen::nonZero<double>()),
      rc::gen::arbitrary<double>()
    ));

  auto objective = *rc::genSizedObjective(ncols, rc::gen::nonZero<double>());

  Solver h;
  h.linear_program().set_objective_sense(*rc::gen::arbitrary<OptimizationType>());
  h.linear_program().add_variables(*gen::container<std::vector<Variable>>(objective.values.size(), gen::arbitrary<Variable>()));
  h.linear_program().set_objective(std::move(objective));
  h.linear_program().add_constraints(std::move(constraints));
  return h;
}

}  // namespace rc

namespace lpint {

struct RawDataLinearProgram {
  OptimizationType sense;
  std::vector<double> values;
  std::vector<int> start_indices;
  std::vector<int> column_indices;
  std::vector<double> lb;
  std::vector<double> ub;
  std::vector<double> objective;
};

// super ugly helper function to generate raw lp data
// values, start indices, col indices, rhs, ord, objective, variable type
template <class Solver>
inline RawDataLinearProgram generate_lp_data(const std::size_t nrows,
                                             const std::size_t ncols) {
  using namespace lpint;

  const auto lp = rc::genLinearProgramSolver<Solver>(nrows, ncols);

  std::vector<double> values, lb, ub;
  std::vector<int> start_indices, col_indices;

  for (const auto &constraint : lp.linear_program().constraints()) {
    const auto &row = constraint.row;
    values.insert(values.end(), row.values().begin(), row.values().end());
    start_indices.push_back(values.size() - row.values().size());
    col_indices.insert(col_indices.end(), row.nonzero_indices().begin(),
                       row.nonzero_indices().end());
    lb.push_back(constraint.lower_bound);
    ub.push_back(constraint.upper_bound);
  }

  const auto& obj = lp.linear_program().objective();
  std::vector<double> objective = obj.values;

  return RawDataLinearProgram{lp.linear_program().optimization_type(),
                              values,
                              start_indices,
                              col_indices,
                              lb,
                              ub,
                              objective};
}

template <class Solver>
inline Solver gen_simple_valid_lp(std::size_t nrows, std::size_t ncols, 
                                  double ub = lpint::LPINT_INFINITY) {
  std::vector<Constraint<double>> constrs;
  for (std::size_t i = 0; i < nrows; i++) {
    auto constr = *rc::genConstraint(rc::genRow(ncols, rc::gen::positive<double>(), true), 
                                                 rc::gen::positive<double>());
    constrs.push_back(std::move(constr));
  }
  const auto& nonzero_indices = constrs.front().row.nonzero_indices();
  const auto count = *std::max_element(nonzero_indices.begin(), nonzero_indices.end()) + 1;

  auto obj = *rc::genSizedObjective(static_cast<std::size_t>(count), 
                                    rc::gen::positive<double>());

  Solver lp;
  lp.linear_program().add_variables(obj.values.size());
  lp.linear_program().set_objective(std::move(obj));
  lp.linear_program().add_constraints(std::move(constrs));
  return lp;
}

}  // namespace lpint

#endif  // LPINTERFACE_GENERATORS_H