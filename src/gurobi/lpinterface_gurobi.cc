#include "lpinterface_gurobi.hpp"

namespace lpint {

GurobiSolver::GurobiSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  // load environment
  GRBloadenv(&gurobi_env_, "");
  // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);

  // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      linear_program_->optimization_type() == OptimizationType::Maximize
          ? GRB_MAXIMIZE
          : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(const GurobiSolver& other) noexcept
    : linear_program_(other.linear_program_) {
  gurobi_model_ = GRBcopymodel(other.gurobi_model_);
  gurobi_env_ = GRBgetenv(gurobi_model_);
}

GurobiSolver& GurobiSolver::operator=(GurobiSolver other) noexcept {
  swap(*this, other);
  return *this;
}

GurobiSolver::GurobiSolver(GurobiSolver&& other) noexcept : GurobiSolver() {
  swap(*this, other);
}

GurobiSolver::~GurobiSolver() {
  // Free Gurobi resources
  GRBfreeenv(GRBgetenv(gurobi_model_));
  GRBfreemodel(gurobi_model_);
}

expected<void, LpError> GurobiSolver::set_parameter(const Param param,
                                                    const int value) {
  switch (param) {
    case Param::GrbOutputFlag:
      GRBsetintparam(GRBgetenv(gurobi_model_), "outputflag", value);
      break;
    case Param::GrbThreads:
      GRBsetintparam(GRBgetenv(gurobi_model_), "threads", value);
      break;
    default:
      return unexpected<LpError>(LpError::UnsupportedParameterError);
  }
  return expected<void, LpError>();
}

expected<void, LpError> GurobiSolver::set_parameter(const Param param,
                                                    const double value) {
  switch (param) {
    case Param::GrbCutoff:
      GRBsetdblparam(GRBgetenv(gurobi_model_), "Cutoff", value);
      break;
    default:
      return unexpected<LpError>(LpError::UnsupportedParameterError);
  }
  return expected<void, LpError>();
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::update_program() {
  return expected<void, LpError>();
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::solve_primal() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::solve_dual() {
  return unexpected<LpError>(LpError::SolveSuccess);
}

const LinearProgramInterface& GurobiSolver::linear_program() const {
  return *linear_program_;
}

LinearProgramInterface& GurobiSolver::linear_program() {
  return *linear_program_;
}

// TODO: actually do something here
expected<std::vector<double>, LpError> GurobiSolver::get_solution() const {
  return unexpected<LpError>(LpError::ModelNotSolvedError);
}

}  // namespace lpint