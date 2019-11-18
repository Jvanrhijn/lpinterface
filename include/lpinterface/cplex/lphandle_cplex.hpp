#ifndef LPINTERFACE_LPHANDLE_CPLEX_H
#define LPINTERFACE_LPHANDLE_CPLEX_H

#include <ilcplex/cplex.h>

#include <memory>
#include <unordered_map>

#include "lpinterface/badge.hpp"
#include "lpinterface/lp.hpp"

namespace lpint {

class CplexSolver;

class LinearProgramHandleCplex : public ILinearProgramHandle {
  using CplexEnv = std::remove_pointer<CPXENVptr>::type;
  using CplexLp = std::remove_pointer<CPXLPptr>::type;

 public:
  LinearProgramHandleCplex(detail::Badge<CplexSolver>,
                           std::shared_ptr<CplexEnv> env);

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

 private:
  std::shared_ptr<CplexEnv> env_;
  std::shared_ptr<CplexLp> lp_;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_CPLEX_H