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
  // special case for iteration limit, since in gurobi it's a double parameter
  if (param == Param::IterationLimit) {
    detail::gurobi_function_checked(
        GRBsetdblparam, GRBgetenv(gurobi_model_.get()), param_dict_.at(param),
        static_cast<double>(value));
  } else {
    detail::gurobi_function_checked(GRBsetintparam,
                                    GRBgetenv(gurobi_model_.get()),
                                    param_dict_.at(param), value);
  }
}

void GurobiSolver::set_parameter(const Param param, const double value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  detail::gurobi_function_checked(GRBsetdblparam,
                                  GRBgetenv(gurobi_model_.get()),
                                  param_dict_.at(param), value);
}

Status GurobiSolver::solve() {
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

  auto num_constraints = lp_handle_.num_constraints();
  solution_.dual.resize(num_constraints);
  detail::gurobi_function_checked(
      GRBgetdblattrarray, gurobi_model_.get(), GRB_DBL_ATTR_PI, 0,
      static_cast<int>(num_constraints), solution_.dual.data());
  return status;
}

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
  if (solution_status() == Status::Optimal) {
    return solution_;
  } else {
    throw ModelNotSolvedException();
  }
}

const std::unordered_map<Param, const char*> GurobiSolver::param_dict_ = {
    {Param::Verbosity, GRB_INT_PAR_OUTPUTFLAG},
    {Param::Threads, GRB_INT_PAR_THREADS},
    {Param::Cutoff, GRB_DBL_PAR_CUTOFF},
    {Param::TimeLimit, GRB_DBL_PAR_TIMELIMIT},
    {Param::IterationLimit, GRB_DBL_PAR_ITERATIONLIMIT},
    {Param::PrimalOrDual, GRB_INT_PAR_METHOD}
};

const std::unordered_map<int, Status> GurobiSolver::status_dict_ = {
    {GRB_LOADED, Status::NoInformation},
    {GRB_OPTIMAL, Status::Optimal},
    {GRB_INFEASIBLE, Status::Infeasible},
    {GRB_INF_OR_UNBD, Status::InfeasibleOrUnbounded},
    {GRB_UNBOUNDED, Status::Unbounded},
    {GRB_CUTOFF, Status::Cutoff},
    {GRB_ITERATION_LIMIT, Status::IterationLimit},
    {GRB_NODE_LIMIT, Status::NodeLimit},
    {GRB_TIME_LIMIT, Status::TimeOut},
    {GRB_SOLUTION_LIMIT, Status::SolutionLimit},
    {GRB_INTERRUPTED, Status::Interrupted},
    {GRB_NUMERIC, Status::NumericFailure},
    {GRB_SUBOPTIMAL, Status::SuboptimalSolution},
    {GRB_INPROGRESS, Status::InProgress},
    {GRB_USER_OBJ_LIMIT, Status::UserObjectiveLimit},
};

}  // namespace lpint