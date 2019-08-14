#ifndef LPINTERFACE_GENERATORS_H
#define LPINTERFACE_GENERATORS_H

#include <iostream>
#include <tuple>

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
    Gen<lpint::Row<T>> rowgen, Gen<T> vgen, Gen<lpint::Ordering> ogen) {
  return gen::build<lpint::Constraint<T>>(
      gen::set(&lpint::Constraint<T>::row, rowgen),
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

// if fixed == true, count is the number of nonzero elements in the row.
// if fixed == false, count is the maximum number of nonzero elements in the
// row.
template <typename T>
inline Gen<lpint::Row<T>> genRow(const std::size_t count, Gen<T> valgen,
                                 bool fixed = false) {
  using namespace lpint;
  std::size_t ncols = fixed ? count : *rc::gen::inRange(0ul, count);
  return gen::construct<Row<T>>(
      rc::gen::container<std::vector<T>>(ncols, std::move(valgen)),
      rc::gen::uniqueCount<std::vector<typename Row<T>::Index>>(
          ncols, rc::gen::inRange(0ul, count)));
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
struct Arbitrary<lpint::Constraint<T>> {
  static Gen<lpint::Constraint<T>> arbitrary() {
    return gen::build<lpint::Constraint<T>>(
        gen::set(&lpint::Constraint<T>::value),
        gen::set(&lpint::Constraint<T>::row),
        gen::set(&lpint::Constraint<T>::ordering));
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
    return gen::element(SparseMatrixType::RowWise,
                        SparseMatrixType::ColumnWise);
  }
};

template <>
struct Arbitrary<lpint::OptimizationType> {
  static Gen<lpint::OptimizationType> arbitrary() {
    return gen::element(lpint::OptimizationType::Maximize,
                        lpint::OptimizationType::Minimize);
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
      rc::gen::arbitrary<OptimizationType>(),
      rc::gen::container<std::vector<Constraint<double>>>(
          nrows, rc::genConstraintWithOrdering(
                     genRow(ncols, rc::gen::arbitrary<double>()),
                     rc::gen::arbitrary<double>(), std::move(genord))),
      rc::genSizedObjective(ncols, std::move(genvt),
                            rc::gen::arbitrary<double>()));
}

inline Gen<std::unique_ptr<lpint::LinearProgram>> genLinearProgramPtr(
    const std::size_t max_nrows, const std::size_t max_ncols,
    Gen<lpint::Ordering> genord, Gen<lpint::VarType> genvt) {
  using namespace lpint;

  const std::size_t nrows =
      *rc::gen::inRange<std::size_t>(1, max_nrows).as("Rows in LP");
  const std::size_t ncols =
      *rc::gen::inRange<std::size_t>(1, max_ncols).as("Columns in LP");

  return gen::makeUnique<LinearProgram>(
      rc::gen::arbitrary<OptimizationType>(),
      rc::gen::container<std::vector<Constraint<double>>>(
          nrows, rc::genConstraintWithOrdering(
                     genRow(ncols, rc::gen::arbitrary<double>()),
                     rc::gen::arbitrary<double>(), std::move(genord))),
      rc::genSizedObjective(ncols, std::move(genvt),
                            rc::gen::arbitrary<double>()));
}

}  // namespace rc

namespace lpint {

// super ugly helper function to generate raw lp data
// values, start indices, col indices, rhs, ord, objective, variable type
inline std::tuple<std::vector<double>, std::vector<int>, std::vector<int>, std::vector<double>, std::vector<lpint::Ordering>, std::vector<double>, std::vector<lpint::VarType>> generate_lp_data(const std::size_t max_nrows, const std::size_t max_ncols_per_row) {
  using namespace lpint;
  const std::size_t nrows =
      *rc::gen::inRange(1ul, max_nrows).as("Number of LP rows");
  const auto ncols_per_row = *rc::gen::container<std::vector<std::size_t>>(
                                  nrows, rc::gen::inRange(1ul, max_ncols_per_row))
                                  .as("LP columns per row");

  const int max_ncols =
      *std::max_element(ncols_per_row.begin(), ncols_per_row.end());

  // generate row values and start indices
  std::vector<double> values;
  std::vector<int> start_indices;
  std::vector<int> col_indices;
  for (const auto& nvals : ncols_per_row) {
    const auto vals = *rc::gen::container<std::vector<double>>(
                           nvals, rc::gen::arbitrary<double>())
                           .as("Row values");
    values.insert(values.end(), vals.begin(), vals.end());
    // TODO check if there is an off-by-one error in here
    start_indices.push_back(values.size() - nvals);
    // generate column indices
    const auto ci = *rc::gen::uniqueCount<std::vector<int>>(
                         nvals, rc::gen::inRange(0, max_ncols))
                         .as("Column indices");
    col_indices.insert(col_indices.end(), ci.begin(), ci.end());
  }

  // generate RHS values
  auto rhs = *rc::gen::container<std::vector<double>>(
                  nrows, rc::gen::arbitrary<double>())
                  .as("RHS values");
  // generate ordering values
  auto ord =
      *rc::gen::container<std::vector<Ordering>>(
           nrows, rc::gen::element(Ordering::LEQ, Ordering::EQ, Ordering::GEQ))
           .as("Constraint orderings");
  // generatee objective values
  auto objective =
      *rc::gen::container<std::vector<double>>(
           static_cast<std::size_t>(max_ncols), rc::gen::nonZero<double>())
           .as("Objective values");
  auto var_type = *rc::gen::container<std::vector<VarType>>(
                       objective.size(), rc::gen::arbitrary<VarType>())
                       .as("Variable types");

  return std::make_tuple(values, start_indices, col_indices, rhs, ord, objective, var_type);
}

}

#endif  // LPINTERFACE_GENERATORS_H