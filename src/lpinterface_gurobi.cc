#include "lpinterface_gurobi.hpp"

namespace lpint {

tl::expected<void, LpError> GurobiSolver::solve_primal() {
  return tl::unexpected<LpError>(LpError::SolveSuccess);
}

}  // namespace lpint