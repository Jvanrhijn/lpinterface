#ifndef LPINTERFACE_GENERATORS_H
#define LPINTERFACE_GENERATORS_H

#include <iostream>
#include "rapidcheck.h"

#include "lpinterface.hpp"

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

template <typename T>
inline Gen<lpint::Row<T>> genRow(const std::size_t count, Gen<T> valgen) {
  using namespace lpint;
  return gen::construct<Row<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::uniqueCount<std::vector<typename Row<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
inline Gen<lpint::Row<T>> genRow(Gen<T> valgen) {
  using namespace lpint;
  const auto count = *rc::gen::arbitrary<std::size_t>();
  return gen::construct<Row<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::uniqueCount<std::vector<typename Row<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
struct Arbitrary<lpint::Row<T>> {
  static Gen<lpint::Row<T>> arbitrary() {
    using namespace lpint;
    return genRow(rc::gen::arbitrary<T>());
  }
};

template <typename T>
inline Gen<lpint::Column<T>> genColumn(const std::size_t count, Gen<T> valgen) {
  using namespace lpint;
  return gen::construct<Column<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::uniqueCount<std::vector<typename Column<T>::Index>>(
          count, rc::gen::inRange(0ul, count)));
}

template <typename T>
inline Gen<lpint::Column<T>> genColumn(Gen<T> valgen) {
  using namespace lpint;
  const auto count = *rc::gen::arbitrary<std::size_t>();
  return gen::construct<Column<T>>(
      rc::gen::container<std::vector<T>>(count, std::move(valgen)),
      rc::gen::uniqueCount<std::vector<typename Column<T>::Index>>(
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
struct Arbitrary<lpint::SparseMatrixType> {
  static Gen<lpint::SparseMatrixType> arbitrary() {
    using namespace lpint;
    return gen::element(SparseMatrixType::RowWise, SparseMatrixType::ColumnWise);
  }
};

inline Gen<lpint::LinearProgram> genLinearProgram(const std::size_t max_nrows,
                                                  const std::size_t max_ncols,
                                                  Gen<lpint::Ordering> genord,
                                                  Gen<lpint::VarType> genvt) {
  using namespace lpint;

  const std::size_t nrows =
      *rc::gen::inRange<std::size_t>(1, max_nrows).as("Rows in LP");
  const std::size_t ncols =
      *rc::gen::inRange<std::size_t>(1, max_ncols).as("Columns in LP");

  return gen::construct<LinearProgram>(
      rc::gen::element(OptimizationType::Maximize, OptimizationType::Minimize),
      rc::gen::container<std::vector<Row<double>>>(
          nrows, genRow(ncols, rc::gen::arbitrary<double>())),
      rc::gen::container<std::vector<Constraint<double>>>(
          nrows, rc::genConstraintWithOrdering(rc::gen::arbitrary<double>(),
                                               std::move(genord))),
      rc::genSizedObjective(ncols, std::move(genvt),
                            rc::gen::arbitrary<double>()));
}

}  // namespace rc

#endif  // LPINTERFACE_GENERATORS_H