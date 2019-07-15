#ifndef LPINTERFACE_TESTS_MOCK_LPSOLVER_H
#define LPINTERFACE_TESTS_MOCK_LPSOLVER_H

#include "common.hpp"
#include "gmock/gmock.h"
#include "lp.hpp"
#include "lpinterface.hpp"

#include <vector>

namespace lpint {

class MockLpSolver : public LinearProgramSolver {
 public:
  MOCK_METHOD0(solve_primal, expected<void, LpError>());
  MOCK_METHOD0(solve_dual, expected<void, LpError>());
  MOCK_METHOD0(update_program, expected<void, LpError>());
  MOCK_CONST_METHOD0(linear_program, const LinearProgram&());
  MOCK_METHOD0(linear_program, LinearProgram&());
  MOCK_CONST_METHOD0(get_solution, expected<std::vector<double>, LpError>());
};

}  // namespace lpint

#endif  // LPINTERFACE_TESTS_MOCK_LPSOLVER_H