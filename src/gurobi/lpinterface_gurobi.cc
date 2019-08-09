#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

namespace lpint {

GurobiSolver::GurobiSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  // load environment
  redirect_stdout();
  GRBloadenv(&gurobi_env_, "");
  restore_stdout();
  // // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  set_parameter(Param::Verbosity, 0);

  // // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      linear_program_->optimization_type() == OptimizationType::Maximize
          ? GRB_MAXIMIZE
          : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(OptimizationType opt_type) {
  redirect_stdout();
  GRBloadenv(&gurobi_env_, "");
  restore_stdout();
  // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  set_parameter(Param::Verbosity, 0);

  // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      opt_type == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(const GurobiSolver& other) noexcept
    : linear_program_(other.linear_program_) {
  redirect_stdout();
  gurobi_model_ = GRBcopymodel(other.gurobi_model_);
  gurobi_env_ = GRBgetenv(gurobi_model_);
  restore_stdout();
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
  auto& matrix = linear_program_->matrix();
  const auto constraints = linear_program_->constraints();
  std::size_t idx = 0;
  if (matrix.type() == SparseMatrixType::RowWise) {
    for (auto& row : matrix) {
      char ord = convert_ordering(constraints[idx].ordering);
      const auto error = GRBaddconstr(
          gurobi_model_, row.num_nonzero(), row.nonzero_indices().data(),
          row.values().data(), ord, constraints[idx].value,
          ("constr" + std::to_string(idx)).c_str());
      if (error != 0) {
        throw GurobiException(error, GRBgeterrormsg(gurobi_env_));
      }
      idx++;
    }
  } else {
    throw MatrixTypeException();
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
  error == GRBgetintattr(gurobi_model_, GRB_INT_ATTR_NUMVARS, &num_vars);
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
    __attribute__((unused)) std::vector<Ordering>&& ord,
    __attribute__((unused)) std::vector<double>&& rhs) {
  throw UnsupportedFeatureException();
}

void GurobiSolver::add_rows(std::vector<double>&& values,
                            std::vector<int>&& start_indices,
                            std::vector<int>&& col_indices,
                            std::vector<Ordering>&& ord,
                            std::vector<double>&& rhs) {
  std::vector<char> ord_grb;
  for (const auto& ordering : ord) {
    ord_grb.push_back(convert_ordering(ordering));
  }
  const auto error = GRBaddconstrs(
      gurobi_model_, start_indices.size(), values.size(), start_indices.data(),
      col_indices.data(), values.data(), ord_grb.data(), rhs.data(), nullptr);
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

void GurobiSolver::redirect_stdout() {
  // TODO: find better way to disable the license thingy
  saved_stdout_ = dup(1);
  close(1);
  new_stdout_ = open("/dev/null", O_WRONLY);
  if (new_stdout_ != 1) {
    throw std::runtime_error("Failed to redirect stdout");
  }
}

void GurobiSolver::restore_stdout() {
  close(new_stdout_);
  new_stdout_ = dup(saved_stdout_);
  if (new_stdout_ != 1) {
    throw std::runtime_error("Failed to redirect stdout");
  }
  close(saved_stdout_);
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

// TODO: extend
constexpr const char* GurobiSolver::translate_parameter(const Param param) {
  switch (param) {
    case (Param::Verbosity):
      return "outputflag";
    case (Param::Threads):
      return "threads";
    case (Param::Cutoff):
      return "Cutoff";
    case (Param::TimeLimit):
      return GRB_DBL_PAR_TIMELIMIT;
    default:
      throw UnsupportedParameterException();
  }
}

}  // namespace lpint