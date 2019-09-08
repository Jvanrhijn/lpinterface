#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

namespace lpint {

GurobiSolver::GurobiSolver()
    : gurobi_env_(detail::create_gurobi_env(), &GRBfreeenv),
      gurobi_model_(detail::create_gurobi_model(gurobi_env_.get()),
                    &GRBfreemodel),
      lp_handle_({}, gurobi_model_, gurobi_env_) {}

GurobiSolver::GurobiSolver(OptimizationType opt_type)
    : gurobi_env_(detail::create_gurobi_env(), &GRBfreeenv),
      gurobi_model_(detail::create_gurobi_model(gurobi_env_.get()),
                    &GRBfreemodel),
      lp_handle_({}, gurobi_model_, gurobi_env_) {
  lp_handle_.set_objective_sense(opt_type);
  // set optimization type
  detail::gurobi_function_checked(
      GRBsetintattr, gurobi_model_.get(), "modelsense",
      opt_type == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
}

bool GurobiSolver::parameter_supported(const Param param) const {
  return param_dict_.count(param);
}

void GurobiSolver::set_parameter(const Param param, const int value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  detail::gurobi_function_checked(GRBsetintparam,
                                  GRBgetenv(gurobi_model_.get()),
                                  param_dict_.at(param), value);
}

void GurobiSolver::set_parameter(const Param param, const double value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  detail::gurobi_function_checked(GRBsetdblparam,
                                  GRBgetenv(gurobi_model_.get()),
                                  param_dict_.at(param), value);
}

Status GurobiSolver::solve_primal() {
  detail::gurobi_function_checked(GRBoptimize, gurobi_model_.get());
  Status status;
  do {
    status = solution_status();
  } while (status == Status::InProgress);

  if (status != Status::Optimal) {
    return status;
  }

  detail::gurobi_function_checked(GRBgetdblattr, gurobi_model_.get(),
                                  GRB_DBL_ATTR_OBJVAL,
                                  &solution_.objective_value);

  auto num_vars = lp_handle_.num_vars();
  solution_.primal.resize(num_vars);
  detail::gurobi_function_checked(GRBgetdblattrarray, gurobi_model_.get(),
                                  GRB_DBL_ATTR_X, 0, static_cast<int>(num_vars),
                                  solution_.primal.data());

  // TODO: this is pretty hacky, find a better way
  // maybe drop support for MIPs entirely
  try {
    auto num_constraints = lp_handle_.num_constraints();
    solution_.dual.resize(num_constraints);
    detail::gurobi_function_checked(
        GRBgetdblattrarray, gurobi_model_.get(), GRB_DBL_ATTR_PI, 0,
        static_cast<int>(num_constraints), solution_.dual.data());
  } catch (const GurobiException& e) {
    constexpr int expected_error = 10005;
    if (e.code() != expected_error) {
      throw e;
    }
  }
  return status;
}

Status GurobiSolver::solve_dual() { throw UnsupportedFeatureException(); }

Status GurobiSolver::solution_status() const {
  int status;
  detail::gurobi_function_checked(GRBgetintattr, gurobi_model_.get(),
                                  GRB_INT_ATTR_STATUS, &status);
  return convert_gurobi_status(status);
}

const ILinearProgramHandle& GurobiSolver::linear_program() const {
  return lp_handle_;
}

ILinearProgramHandle& GurobiSolver::linear_program() { return lp_handle_; }

const Solution<double>& GurobiSolver::get_solution() const {
  return solution_status() == Status::Optimal ? solution_
                                              : throw ModelNotSolvedException();
}

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
  detail::gurobi_function_checked(GRBaddrangeconstrs, gurobi_model_.get(),
                                  start_indices.size(), values.size(),
                                  start_indices.data(), col_indices.data(),
                                  values.data(), lb.data(), ub.data(), nullptr);
}
void GurobiSolver::add_variables(std::vector<double>&& objective_values,
                                 std::vector<VarType>&& var_types) {
  detail::gurobi_function_checked(
      GRBaddvars, gurobi_model_.get(), objective_values.size(), 0, nullptr,
      nullptr, nullptr, objective_values.data(), nullptr, nullptr,
      LinearProgramHandleGurobi::convert_variable_type(var_types).data(),
      nullptr);
  lp_handle_.set_num_vars({}, objective_values.size());
}

const std::unordered_map<Param, const char*> GurobiSolver::param_dict_ = {
    {Param::Verbosity, GRB_INT_PAR_OUTPUTFLAG},
    {Param::Threads, GRB_INT_PAR_THREADS},
    {Param::Cutoff, GRB_DBL_PAR_CUTOFF},
    {Param::TimeLimit, GRB_DBL_PAR_TIMELIMIT},
    {Param::IterationLimit, GRB_DBL_PAR_ITERATIONLIMIT},
};

}  // namespace lpint