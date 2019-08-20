#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

namespace lpint {

GurobiSolver::GurobiSolver(std::unique_ptr<LinearProgramInterface>&& lp)
    : saved_stdout_(0),
      new_stdout_(0),
      linear_program_(std::move(lp)),
      gurobi_env_(nullptr),
      gurobi_model_(nullptr) {
  // load environment
  GRBloadenv(&gurobi_env_, "");
  // // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  if (const auto error = GRBsetintparam(
          GRBgetenv(gurobi_model_), translate_parameter(Param::Verbosity), 0)) {
    throw GurobiException(error);
  }

  // // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      linear_program_->optimization_type() == OptimizationType::Maximize
          ? GRB_MAXIMIZE
          : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(OptimizationType opt_type)
    : saved_stdout_(0),
      new_stdout_(0),
      linear_program_(),
      gurobi_env_(nullptr),
      gurobi_model_(nullptr) {
  GRBloadenv(&gurobi_env_, "");
  // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  if (const auto error = GRBsetintparam(
          GRBgetenv(gurobi_model_), translate_parameter(Param::Verbosity), 0)) {
    throw GurobiException(error);
  }

  // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      opt_type == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(GurobiSolver&& other) noexcept : GurobiSolver() {
  swap(*this, other);
}

GurobiSolver& GurobiSolver::operator=(GurobiSolver&& other) noexcept {
  swap(*this, other);
  return *this;
}

GurobiSolver::~GurobiSolver() {
  // Free Gurobi resources
  GRBfreeenv(GRBgetenv(gurobi_model_));
  GRBfreemodel(gurobi_model_);
}

void GurobiSolver::set_parameter(const Param param, const int value) {
  if (const auto error = GRBsetintparam(GRBgetenv(gurobi_model_),
                                        translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::set_parameter(const Param param, const double value) {
  if (const auto error = GRBsetdblparam(GRBgetenv(gurobi_model_),
                                        translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::update_program() {
  if (!linear_program_->is_initialized()) {
    throw LinearProgramNotInitializedException();
  }
  // first add variables to Gurobi
  auto objective = linear_program_->objective();
  const std::size_t num_vars = linear_program_->num_vars();
  auto vt = convert_variable_type(objective.variable_types);
  auto err =
      GRBaddvars(gurobi_model_, num_vars, 0, nullptr, nullptr, nullptr,
                 objective.values.data(), nullptr, nullptr, vt.data(), nullptr);
  if (err != 0) {
    throw GurobiException(err, GRBgeterrormsg(gurobi_env_));
  }
  // set constraints
  auto& constraints = linear_program_->constraints();
  for (auto& constraint : constraints) {
    const auto error = GRBaddrangeconstr(gurobi_model_, constraint.row.num_nonzero(),
                                    constraint.row.nonzero_indices().data(),
                                    constraint.row.values().data(), constraint.lower_bound,
                                    constraint.upper_bound, nullptr);
    if (error != 0) {
      throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
    }
  }
}

Status GurobiSolver::solve_primal() {
  auto error = GRBoptimize(gurobi_model_);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
  }
  Status status;
  do {
    status = solution_status();
  } while (status == Status::InProgress);

  if (status != Status::Optimal) {
    return status;
  }

  error = GRBgetdblattr(gurobi_model_, GRB_DBL_ATTR_OBJVAL,
                        &solution_.objective_value);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
  }

  int num_vars;
  if (linear_program_ != nullptr) {
    num_vars = static_cast<int>(linear_program_->num_vars());
  } else {
    error = GRBgetintattr(gurobi_model_, GRB_INT_ATTR_NUMVARS, &num_vars);
    if (error) {
      throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
    }
  }

  solution_.primal.resize(static_cast<std::size_t>(num_vars));

  error = GRBgetdblattrarray(gurobi_model_, GRB_DBL_ATTR_X, 0, num_vars,
                             solution_.primal.data());
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
  }
  return status;
}

Status GurobiSolver::solve_dual() { throw UnsupportedFeatureException(); }

Status GurobiSolver::solution_status() const {
  int status;
  const auto error = GRBgetintattr(gurobi_model_, GRB_INT_ATTR_STATUS, &status);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
  }
  return convert_gurobi_status(status);
}

const LinearProgramInterface& GurobiSolver::linear_program() const {
  return *linear_program_;
}

LinearProgramInterface& GurobiSolver::linear_program() {
  return *linear_program_;
}

const Solution<double>& GurobiSolver::get_solution() const { return solution_; }

void GurobiSolver::add_columns(
    __attribute__((unused)) std::vector<double>&& values,
    __attribute__((unused)) std::vector<int>&& start_indices,
    __attribute__((unused)) std::vector<int>&& row_indices,
    __attribute__((unused)) std::vector<double>&& lb,
    __attribute__((unused)) std::vector<double>&& ub) {
  throw UnsupportedFeatureException();
}

void GurobiSolver::add_rows(std::vector<double>&& values,
                            std::vector<int>&& start_indices,
                            std::vector<int>&& col_indices,
                            std::vector<double>&& lb,
                            std::vector<double>&& ub) {
  const auto error = GRBaddrangeconstrs(
      gurobi_model_, start_indices.size(), values.size(), start_indices.data(),
      col_indices.data(), values.data(), lb.data(), ub.data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}
void GurobiSolver::add_variables(std::vector<double>&& objective_values,
                                 std::vector<VarType>&& var_types) {
  const auto error =
      GRBaddvars(gurobi_model_, objective_values.size(), 0, nullptr, nullptr,
                 nullptr, objective_values.data(), nullptr, nullptr,
                 convert_variable_type(var_types).data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}

std::vector<char> GurobiSolver::convert_variable_type(
    const std::vector<VarType>& var_types) {
  const auto num_vars = var_types.size();
  std::vector<char> value_type(num_vars);
  for (std::size_t i = 0; i < num_vars; i++) {
    char vtype;
    switch (var_types[i]) {
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
        throw UnsupportedVariableTypeException();
    }
    value_type[i] = vtype;
  }
  return value_type;
}

}  // namespace lpint