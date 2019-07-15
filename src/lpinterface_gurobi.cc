#include "lpinterface_gurobi.hpp"

namespace lpint {

GurobiSolver::GurobiSolver(LinearProgram&& lp) 
  : linear_program_(&lp)
{
}

expected<void, LpError> GurobiSolver::solve_primal() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

}  // namespace lpint