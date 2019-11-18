#include "lpinterface/cplex/lpinterface_cplex.hpp"
#include "lpinterface/cplex/lputil_cplex.hpp"

// macro magic to make cplex behave well (thanks Alberto Santini)
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// end magic

namespace lpint {

// Destructor for env is kind of sketchy, may segfault
CplexSolver::CplexSolver(OptimizationType opt_type) 
    : env_(CPXopenCPLEX(&status_), [](CPXENVptr p) {CPXcloseCPLEX(&p); }),
      lp_handle_({}, env_)
{
  lp_handle_.set_objective_sense(opt_type);
}

CplexSolver::CplexSolver() 
    : env_(CPXopenCPLEX(&status_), [](CPXENVptr p) {CPXcloseCPLEX(&p); }),
      lp_handle_({}, env_)
{
  lp_handle_.set_objective_sense(OptimizationType::Maximize);
}

bool CplexSolver::parameter_supported(const Param param) const {
  return int_param_dict_.count(param);
}

void CplexSolver::set_parameter(const Param param, const int value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  detail::cplex_function_checked(CPXsetintparam, env_.get(), int_param_dict_.at(param), value);
}

void CplexSolver::set_parameter(const Param param, const double value) {
  throw NotImplementedError();
}

Status CplexSolver::solve() {
  throw NotImplementedError();
}

Status CplexSolver::solution_status() const {
  throw NotImplementedError();
}

const ILinearProgramHandle& CplexSolver::linear_program() const {
  return lp_handle_;
}

ILinearProgramHandle& CplexSolver::linear_program() {
  return lp_handle_;
}

const Solution<double>& CplexSolver::get_solution() const {
  throw NotImplementedError();
}

const std::unordered_map<Param, int> CplexSolver::int_param_dict_ =
{
  {Param::Verbosity, CPXPARAM_ScreenOutput}
};

}