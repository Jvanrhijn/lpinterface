#include "lpinterface_gurobi.hpp"

namespace lpint {

expected<void, LpError> GurobiSolver::solve_primal() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

}  // namespace lpint