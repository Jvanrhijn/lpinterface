#ifndef LPINTERFACE_TESTS_MOCK_LPSOLVER_H
#define LPINTERFACE_TESTS_MOCK_LPSOLVER_H

#include "lpinterface.hpp"

#include <vector>

namespace lpint {

class MockLpSolver : public LinearProgramSolver {
 public:
  MOCK_METHOD0(solve_primal, Status());
  MOCK_METHOD0(solve_dual, Status());
  MOCK_METHOD0(update_program, void());
  MOCK_CONST_METHOD0(linear_program, const LinearProgramInterface&());
  MOCK_METHOD0(linear_program, LinearProgramInterface&());
  MOCK_CONST_METHOD0(get_solution, const Solution<double>&());
  MOCK_METHOD2(set_parameter, void(const Param, const int));
  MOCK_METHOD2(set_parameter, void(const Param, const double));
};

}  // namespace lpint

#endif  // LPINTERFACE_TESTS_MOCK_LPSOLVER_H
