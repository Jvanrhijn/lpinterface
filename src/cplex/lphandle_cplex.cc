#include "lpinterface/cplex/lphandle_cplex.hpp"
#include "lpinterface/cplex/lputil_cplex.hpp"

namespace lpint {

// destructor for  LP is a little sketch
LinearProgramHandleCplex::LinearProgramHandleCplex(detail::Badge<CplexSolver>,
                                                   std::shared_ptr<CplexEnv> env)
    : env_(env), 
    lp_(detail::create_cplex_problem(env_), [&](CplexLp* lp) {CPXfreeprob(env_.get(), &lp); })
{
}

LinearProgramHandleCplex::~LinearProgramHandleCplex() {
  // TODO: destruct pointers to problem data via free_and_null
}

std::size_t LinearProgramHandleCplex::num_vars() const {
  return static_cast<std::size_t>(CPXgetnumcols(env_.get(), lp_.get()));
}

std::size_t LinearProgramHandleCplex::num_constraints() const {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::set_objective_sense(const OptimizationType objsense) {
  detail::cplex_function_checked(CPXchgobjsen, env_.get(), lp_.get(), 
    objsense == OptimizationType::Maximize?
      CPX_MAX : CPX_MIN 
   );
}

Variable LinearProgramHandleCplex::variable(std::size_t i) const {
  throw NotImplementedError();
}

std::vector<Variable> LinearProgramHandleCplex::variables() const {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::add_variables(const std::vector<Variable>& vars) {
  std::vector<double> lbs(vars.size());
  std::vector<double> ubs(vars.size());

  std::transform(vars.begin(), vars.end(), lbs.begin(), 
    [](const Variable& var) { return var.lower(); });
  std::transform(vars.begin(), vars.end(), ubs.begin(), 
    [](const Variable& var) { return var.upper(); });

  detail::cplex_function_checked(CPXaddcols, env_.get(), lp_.get(), vars.size(), 0, nullptr, nullptr, nullptr, nullptr,
    lbs.data(), ubs.data(), nullptr);
}

void LinearProgramHandleCplex::add_variables(const std::size_t num_vars) {
  add_variables(std::vector<Variable>(num_vars));
}

void LinearProgramHandleCplex::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::remove_variable(const std::size_t i) {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::remove_constraint(std::size_t i) {
  throw NotImplementedError();
}

OptimizationType LinearProgramHandleCplex::optimization_type() const {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::set_objective(const Objective<double>& objective) {
  throw NotImplementedError();
}

Constraint<double> LinearProgramHandleCplex::constraint(std::size_t i) const {
  throw NotImplementedError();
}

std::vector<Constraint<double>> LinearProgramHandleCplex::constraints() const {
  throw NotImplementedError();
}

Objective<double> LinearProgramHandleCplex::objective() const {
  throw NotImplementedError();
}

}