#ifndef LPINTERFACE_TESTS_MOCK_LP_H
#define LPINTERFACE_TESTS_MOCK_LP_H

#include "lpinterface.hpp"

#include "gmock/gmock.h"

#include <vector>

namespace lpint {

class MockLinearProgram : public LinearProgramInterface {
 public:
  MOCK_CONST_METHOD0(num_vars, std::size_t());
  MOCK_CONST_METHOD0(constraints, const std::vector<Constraint<double>>&());
  MOCK_METHOD0(constraints, std::vector<Constraint<double>>&());
  MOCK_METHOD1(add_constraints, void(std::vector<Constraint<double>>&&));
  MOCK_CONST_METHOD0(optimization_type, OptimizationType());
  MOCK_METHOD1(set_objective, void(const Objective<double>&));
  MOCK_CONST_METHOD0(objective, const Objective<double>&());
  MOCK_METHOD0(objective, Objective<double>&());
  MOCK_CONST_METHOD0(is_initialized, bool());
};

}  // namespace lpint

#endif  // LPINTERFACE_TESTS_MOCK_LP_H
