#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

namespace lpint {

GurobiSolver::GurobiSolver(OptimizationType opt_type)
    : gurobi_env_(detail::create_gurobi_env(), &GRBfreeenv),
      gurobi_model_(detail::create_gurobi_model(gurobi_env_.get()), &GRBfreemodel),
      lp_handle_(gurobi_model_, gurobi_env_) { 
  lp_handle_.set_objective_sense(opt_type);
  if (const auto error = GRBsetintparam(
          GRBgetenv(gurobi_model_.get()), translate_parameter(Param::Verbosity), 0)) {
    throw GurobiException(error);
  }
  // set optimization type
  GRBsetintattr(
      gurobi_model_.get(), "modelsense",
      opt_type == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
}

void GurobiSolver::set_parameter(const Param param, const int value) {
  if (const auto error = GRBsetintparam(GRBgetenv(gurobi_model_.get()),
                                        translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::set_parameter(const Param param, const double value) {
  if (const auto error = GRBsetdblparam(GRBgetenv(gurobi_model_.get()),
                                        translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::update_program() {
}

Status GurobiSolver::solve_primal() {
  auto error = GRBoptimize(gurobi_model_.get());
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_.get()));
  }
  Status status;
  do {
    status = solution_status();
  } while (status == Status::InProgress);

  if (status != Status::Optimal) {
    return status;
  }

  error = GRBgetdblattr(gurobi_model_.get(), GRB_DBL_ATTR_OBJVAL,
                        &solution_.objective_value);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_.get()));
  }

  auto num_vars = static_cast<int>(lp_handle_.num_vars());
  error = GRBgetintattr(gurobi_model_.get(), GRB_INT_ATTR_NUMVARS, &num_vars);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_.get()));
  }

  solution_.primal.resize(static_cast<std::size_t>(num_vars));

  error = GRBgetdblattrarray(gurobi_model_.get(), GRB_DBL_ATTR_X, 0, num_vars,
                             solution_.primal.data());
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_.get()));
  }
  return status;
}

Status GurobiSolver::solve_dual() { throw UnsupportedFeatureException(); }

Status GurobiSolver::solution_status() const {
  int status;
  const auto error = GRBgetintattr(gurobi_model_.get(), GRB_INT_ATTR_STATUS, &status);
  if (error) {
    throw GurobiException(error, GRBgeterrormsg(gurobi_env_.get()));
  }
  return convert_gurobi_status(status);
}

const ILinearProgramHandle& GurobiSolver::linear_program() const {
  return lp_handle_;
}

ILinearProgramHandle& GurobiSolver::linear_program() {
  return lp_handle_;
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
      gurobi_model_.get(), start_indices.size(), values.size(), start_indices.data(),
      col_indices.data(), values.data(), lb.data(), ub.data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}
void GurobiSolver::add_variables(std::vector<double>&& objective_values,
                                 std::vector<VarType>&& var_types) {
  const auto error =
      GRBaddvars(gurobi_model_.get(), objective_values.size(), 0, nullptr, nullptr,
                 nullptr, objective_values.data(), nullptr, nullptr,
                 LinearProgramHandleGurobi::convert_variable_type(var_types).data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}


}  // namespace lpint