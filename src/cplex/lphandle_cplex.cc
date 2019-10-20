#include "lpinterface/cplex/lphandle_cplex.hpp"

namespace lpint {

LinearProgramHandleCplex::LinearProgramHandleCplex(detail::Badge<CplexSolver>,
                                                   IloEnv env,
                                                   std::shared_ptr<IloCplex> cplex) 
    : model_(env), cplex_(cplex)
{}

std::size_t LinearProgramHandleCplex::num_vars() const {
  throw NotImplementedError();
}

std::size_t LinearProgramHandleCplex::num_constraints() const {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::set_objective_sense(const OptimizationType objsense) {
  throw NotImplementedError();
}

Variable LinearProgramHandleCplex::variable(std::size_t i) const {
  throw NotImplementedError();
}

std::vector<Variable> LinearProgramHandleCplex::variables() const {
  throw NotImplementedError();
}

void LinearProgramHandleCplex::add_variables(const std::vector<Variable>& vars) {
  for (const auto& var: vars) {
    vars_.add(IloNumVar(model_.getEnv(), var.lower(), var.upper()));
  }
}

void LinearProgramHandleCplex::add_variables(const std::size_t num_vars) {
  add_variables(std::vector<Variable>(num_vars));
}

void LinearProgramHandleCplex::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  IloRangeArray constrs(model_.getEnv());
  for (const auto& c : constraints) {
    IloNumExpr expr;
    const auto& nonzeros = c.row.nonzero_indices();
    const auto& coeffs = c.row.values();
    for (std::size_t i = 0; i < c.row.num_nonzero(); i++) {
      expr += coeffs[static_cast<std::size_t>(nonzeros[i])] 
          * vars_[nonzeros[i]];
    }
    constrs.add(c.lower_bound <= expr <= c.upper_bound);
  }
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