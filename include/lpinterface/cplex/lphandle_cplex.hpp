#ifndef LPINTERFACE_LPHANDLE_CPLEX_H
#define LPINTERFACE_LPHANDLE_CPLEX_H

#include "lpinterface/badge.hpp"
#include "lpinterface/lp.hpp"

// Magic tricks to have CPLEX behave well:
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// End magic tricks

namespace lpint {

class LinearProgramHandleCplex : public ILinearProgramHandle {

 public:
  LinearProgramHandleCplex(detail::Badge<LinearProgramHandleCplex>);

  std::size_t num_vars() const override;

  std::size_t num_constraints() const override;

  void set_objective_sense(const OptimizationType objsense) override;

  Variable variable(std::size_t i) const override;

  std::vector<Variable> variables() const override;

  void add_variables(const std::vector<Variable>& vars) override;

  void add_variables(const std::size_t num_vars) override;

  void add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  void remove_variable(const std::size_t i) override;

  void remove_constraint(std::size_t i) override;

  OptimizationType optimization_type() const override;

  void set_objective(const Objective<double>& objective) override;

  Constraint<double> constraint(std::size_t i) const override;

  std::vector<Constraint<double>> constraints() const override;

  Objective<double> objective() const override;

};

} // lpint

#endif // LPINTERFACE_LPHANDLE_CPLEX_H