#include "lpinterface_gurobi.hpp"
#include <iostream>

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
  // first add variables to Gurobi
  auto objective = linear_program_->objective();
  std::size_t num_vars = objective.values.size();
  std::vector<double> obj(objective.values.begin(), objective.values.end());
  std::vector<char> value_type(num_vars);
  for (std::size_t i = 0; i < num_vars; i++) {
    char vtype;
    switch (objective.variable_types[i]) {
      case VarType::Binary:
        vtype = GRB_BINARY;
        break;
      case VarType::Integer:
        vtype = GRB_INTEGER;
        break;
      case VarType::Real:
        vtype = GRB_CONTINUOUS;
        break;
      case VarType::SemiReal:
        vtype = GRB_SEMICONT;
        break;
      case VarType::SemiInteger:
        vtype = GRB_SEMIINT;
        break;
      default:
        return unexpected<LpError>(LpError::UnsupportedVariableTypeError);
    }
    value_type[i] = vtype;
  }
  auto err =
      GRBaddvars(gurobi_model_, num_vars, 0, nullptr, nullptr, nullptr,
                 obj.data(), nullptr, nullptr, value_type.data(), nullptr);
  if (err) {
    return unexpected<LpError>(convert_gurobi_error(err));
  }
  // set constraints
  auto matrix = linear_program_->matrix();
  auto constraints = linear_program_->constraints();
  std::size_t idx = 0;
  if (matrix.type() == SparseMatrixType::RowWise) {
    for (const auto& row : matrix) {
      char ord;
      switch (constraints[idx].ordering) {
        case Ordering::LEQ:
          ord = GRB_LESS_EQUAL;
          break;
        case Ordering::GEQ:
          ord = GRB_GREATER_EQUAL;
          break;
        case Ordering::EQ:
          ord = GRB_EQUAL;
          break;
        default:
          return unexpected<LpError>(LpError::UnsupportedConstraintError);
      }
      // need to do this since gurobi wants int* for indices
      // for some ungodly reason
      std::vector<int> nonzero_indices(row.nonzero_indices().begin(),
                                       row.nonzero_indices().end());
      // need to do this since gurobi takes double* rather than const double*
      // const
      std::vector<double> values(row.values().begin(), row.values().end());
      auto error =
          GRBaddconstr(gurobi_model_, row.num_nonzero(), nonzero_indices.data(),
                       values.data(), ord, constraints[idx].value,
                       ("constr" + std::to_string(idx)).c_str());
      if (error != 0) {
        return unexpected<LpError>(convert_gurobi_error(error));
      }
      idx++;
    }
  } else {
    return unexpected<LpError>(LpError::MatrixTypeError);
  }
  return expected<void, LpError>();
}

// TODO: actually do something here
expected<void, LpError> GurobiSolver::solve_primal() {
  auto error = GRBoptimize(gurobi_model_);
  if (error) {
    return unexpected<LpError>(convert_gurobi_error(error));
  }
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