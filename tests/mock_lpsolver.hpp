#ifndef LPINTERFACE_TESTS_MOCK_LPSOLVER_H
#define LPINTERFACE_TESTS_MOCK_LPSOLVER_H

#include "lpinterface.hpp"
#include "lp.hpp"
#include "gmock/gmock.h"

#include <vector>

namespace lpint {

class MockLpSolver : public LinearProgramSolver {

  public:
    MOCK_METHOD0(solve_primal, LpError());
    MOCK_METHOD0(solve_dual, LpError());
    MOCK_CONST_METHOD0(linear_program, const LinearProgram&());
    MOCK_METHOD0(linear_program, LinearProgram&());
    MOCK_METHOD0(get_solution, std::vector<double>());
};

}

#endif // LPINTERFACE_TESTS_MOCK_LPSOLVER_H