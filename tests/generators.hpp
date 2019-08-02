#ifndef LPINTERFACE_GENERATORS_H
#define LPINTERFACE_GENERATORS_H

#include <iostream>
#include "rapidcheck.h"

// hack unique container generators with count parameter into rapidcheck
namespace rc {

namespace gen {

template <typename Container, typename T, typename F>
Gen<Container> uniqueByCount(std::size_t count, Gen<T> gen, F &&f) {
  using Strategy = detail::UniqueContainerStrategy<Decay<F>>;
  detail::ContainerHelper<Container, Strategy> helper(
      Strategy(std::forward<F>(f)));

  return [=](const Random &random, int size) {
    return helper.generate(count, random, size, gen);
  };
}

template <typename Container, typename T>
Gen<Container> uniqueCount(std::size_t count, Gen<T> gen) {
  return uniqueByCount<Container>(count, std::move(gen),
                                  [](const T &x) -> const T & { return x; });
}

}  // namespace gen

}  // namespace rc

namespace rc {

template <typename T>
inline Gen<std::vector<T>> genUniqueVector(std::size_t size, Gen<T> gen) {
  return gen::suchThat(gen::unique<std::vector<T>>(gen),
                       [&](std::vector<T> v) { return v.size() == size; });
}

template <>
struct Arbitrary<lpint::Ordering> {
  static Gen<lpint::Ordering> arbitrary() {
    return gen::element(lpint::Ordering::EQ, lpint::Ordering::GEQ,
                        lpint::Ordering::GT, lpint::Ordering::LEQ,
                        lpint::Ordering::LT);
  }
};

template <typename T>
struct Arbitrary<lpint::Constraint<T>> {
  static Gen<lpint::Constraint<T>> arbitrary() {
    return gen::build<lpint::Constraint<T>>(
        gen::set(&lpint::Constraint<T>::value),
        gen::set(&lpint::Constraint<T>::ordering));
  }
};

template <>
struct Arbitrary<lpint::VarType> {
  static Gen<lpint::VarType> arbitrary() {
    return gen::element(lpint::VarType::Binary, lpint::VarType::Integer,
                        lpint::VarType::Real, lpint::VarType::SemiInteger,
                        lpint::VarType::SemiReal);
  }
};

template <typename T>
struct Arbitrary<lpint::Objective<T>> {
  static Gen<lpint::Objective<T>> arbitrary() {
    return gen::build<lpint::Objective<T>>(
        gen::set(&lpint::Objective<T>::values),
        gen::set(&lpint::Objective<T>::variable_types));
  }
};

template <typename T>
struct Arbitrary<lpint::Row<T>> {
  static Gen<lpint::Row<T>> arbitrary() {}
};

template <typename T>
inline Gen<lpint::Constraint<T>> genConstraintWithOrdering(
    Gen<T> vgen, Gen<lpint::Ordering> ogen) {
  return gen::build<lpint::Constraint<T>>(
      gen::set(&lpint::Constraint<T>::ordering, ogen),
      gen::set(&lpint::Constraint<T>::value, vgen));
}

template <typename T>
inline Gen<lpint::Objective<T>> genSizedObjective(std::size_t size,
                                                  Gen<lpint::VarType> vtgen,
                                                  Gen<T> valgen) {
  return gen::build<lpint::Objective<T>>(
      gen::set(&lpint::Objective<T>::values,
               gen::container<std::vector<T>>(size, valgen)),
      gen::set(&lpint::Objective<T>::variable_types,
               gen::container<std::vector<lpint::VarType>>(size, vtgen)));
}

// TODO: refactor into an Arbitrary instance or a true Gen<LinearProgram>
inline lpint::LinearProgram generateLinearProgram(const std::size_t nrows,
                                                  const std::size_t ncols,
                                                  lpint::OptimizationType opt_type) {
  using namespace lpint;
  // construct an LP
  LinearProgram lp(opt_type, SparseMatrixType::RowWise);

  // generate objective
  const auto objective = *rc::genSizedObjective(
      ncols, rc::gen::just(VarType::Real), rc::gen::arbitrary<double>()).as("Objective");
  lp.set_objective(objective);

  // generate constraints
  auto constraints = *rc::gen::container<std::vector<Constraint<double>>>(
      nrows, rc::genConstraintWithOrdering(
                 rc::gen::arbitrary<double>(),
                 rc::gen::element(Ordering::LEQ, Ordering::GEQ))).as("Constraints");

  // generate constraint matrix
  std::vector<Row<double>> rows;
  for (std::size_t i = 0; i < nrows; i++) {
    auto values = *rc::gen::container<std::vector<double>>(
                       ncols, rc::gen::arbitrary<double>())
                       .as("Row values");
    auto indices = *rc::gen::uniqueCount<std::vector<std::size_t>>(
                        ncols, rc::gen::inRange(0ul, values.size()))
                        .as("Row indices");
    rows.emplace_back(values, indices);
  }

  lp.add_rows(std::move(rows));
  lp.add_constraints(std::move(constraints));

  return lp;
}

}  // namespace rc

#endif  // LPINTERFACE_GENERATORS_H