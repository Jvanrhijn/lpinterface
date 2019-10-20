#include "lpinterface/cplex/lpinterface_cplex.hpp"

// macro magic to make cplex behave well (thanks Alberto Santini)
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// end magic

namespace lpint {

CplexSolver::CplexSolver(OptimizationType opt_type) 
    : cplex_(std::make_shared<IloCplex>(env_)), lp_handle_({}, env_, cplex_)
{
  lp_handle_.set_objective_sense(opt_type);
}

CplexSolver::CplexSolver() 
    : cplex_(std::make_shared<IloCplex>(env_)), lp_handle_({}, env_, cplex_)
{
  lp_handle_.set_objective_sense(OptimizationType::Maximize);
}

bool CplexSolver::parameter_supported(const Param param) const {
  throw NotImplementedError();
}

void CplexSolver::set_parameter(const Param param, const int value) {
  throw NotImplementedError();
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
  throw NotImplementedError();
}

ILinearProgramHandle& CplexSolver::linear_program() {
  throw NotImplementedError();
}

const Solution<double>& CplexSolver::get_solution() const {
  throw NotImplementedError();
}

}