#include <vector>

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"

#include "generators.hpp"

using namespace lpint;

RC_GTEST_PROP(DataObjects, MatrixEntryThrowsIfDuplicateNonzeros, ()) {
  const auto values = *rc::gen::container<std::vector<double>>(
      *rc::gen::inRange(uint8_t(2), std::numeric_limits<uint8_t>::max()),
      rc::gen::arbitrary<double>());
  auto indices = *rc::gen::unique<std::vector<Row<double>::Index>>(
      values.size(), rc::gen::inRange(0ul, values.size()));

  const auto from = *rc::gen::inRange(0ul, values.size() / 2);
  const auto to = *rc::gen::inRange(from + 1, values.size());
  indices[to] = indices[from];  // make a duplicate index

  RC_ASSERT_THROWS_AS(
      MatrixEntry<double>(std::move(values), std::move(indices)),
      InvalidMatrixEntryException);
}

RC_GTEST_PROP(DataObjects, MatrixEntryThrowsIfDimensionsMismatched,
              (std::vector<double> vs,
               std::vector<MatrixEntry<double>::Index> is)) {
  if (vs.size() != is.size()) {
    RC_ASSERT_THROWS_AS(MatrixEntry<double>(std::move(vs), std::move(is)),
                        MismatchedDimensionsException);
  }
}

RC_GTEST_PROP(DataObjects, VariableThrowsIfLowerBoundGreaterThanUpperBound, 
              (double x1, double x2)) {
  double lb = std::max(x1, x2);
  double ub = std::min(x1, x2);
  if (lb != ub) {
    RC_ASSERT_THROWS_AS(Variable(lb, ub), InvalidVariableBoundsException);
  }
}
