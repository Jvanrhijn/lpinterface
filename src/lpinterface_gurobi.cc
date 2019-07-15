#include "lpinterface_gurobi.hpp"

namespace lpint {

GurobiSolver::GurobiSolver(LinearProgram&& lp) 
  : linear_program_(&lp)
{
  auto env = gurobi_env_.get();
  auto model = gurobi_model_.get();

  // load environment 
  GRBloadenv(&env, "");
  // allocate Gurobi model
  GRBnewmodel(env, &model, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr);
}

GurobiSolver::~GurobiSolver() {
  // Free Gurobi resources
  GRBfreemodel(gurobi_model_.get());
  GRBfreeenv(gurobi_env_.get());
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::solve_primal() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::solve_dual() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

const LinearProgram& GurobiSolver::linear_program() const {
  return *linear_program_;
}

LinearProgram& GurobiSolver::linear_program() {
  return *linear_program_;
}

// TODO: actually do something here
expected<std::vector<double>, LpError> GurobiSolver::get_solution() const {
  return expected<std::vector<double>, LpError>(std::vector<double>());
}

}  // namespace lpint