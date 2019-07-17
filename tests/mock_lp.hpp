#ifndef LPINTERFACE_TESTS_MOCK_LP_H
#define LPINTERFACE_TESTS_MOCK_LP_H

#include "common.hpp"
#include "data_objects.hpp"
#include "errors.hpp"
#include "lp.hpp"

#include "gmock/gmock.h"

#include <vector>

namespace lpint {

class MockLinearProgram : public LinearProgramInterface {
 public:
  MOCK_METHOD1(add_columns,
               expected<void, LpError>(const std::vector<Column<double>>&));
  MOCK_METHOD1(add_rows,
               expected<void, LpError>(const std::vector<Row<double>>&));
  MOCK_METHOD1(set_matrix,
               expected<void, LpError>(const SparseMatrix<double>&));
  MOCK_CONST_METHOD0(matrix, const SparseMatrix<double>&());
  MOCK_CONST_METHOD0(constraints, const std::vector<Constraint<double>>&());
  MOCK_METHOD1(add_constraints, expected<void, LpError>(const std::vector<Constraint<double>>&));
  MOCK_CONST_METHOD0(optimization_type, OptimizationType());
  MOCK_METHOD1(set_objective, expected<void, LpError>(const Objective<double>&));
  MOCK_CONST_METHOD0(objective, const Objective<double>&());
};

}  // namespace lpint

#endif  // LPINTERFACE_TESTS_MOCK_LP_H