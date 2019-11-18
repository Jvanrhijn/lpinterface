#ifndef LPINTERFACE_LPHANDLE_SOPLEX_H
#define LPINTERFACE_LPHANDLE_SOPLEX_H

#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "lpinterface/badge.hpp"
#include "lpinterface/detail/util.hpp"
#include "lpinterface/lp.hpp"
#include "soplex.h"

namespace lpint {

class SoplexSolver;

class LinearProgramHandleSoplex : public ILinearProgramHandle {
 public:
  LinearProgramHandleSoplex(detail::Badge<SoplexSolver>,
                            std::shared_ptr<soplex::SoPlex> soplex)
      : soplex_(soplex) {
    std::iota(permutation_.begin(), permutation_.end(), 0);
    inverse_permutation_ = detail::inverse_permutation(permutation_);
    std::iota(permutation_vars_.begin(), permutation_vars_.end(), 0);
    inverse_permutation_vars_ = detail::inverse_permutation(permutation_vars_);
  }

  Variable variable(std::size_t i) const override;

  std::vector<Variable> variables() const override;

  void add_variables(const std::vector<Variable>& vars) override;

  void add_variables(std::size_t num_vars) override;

  void add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  void remove_variable(const std::size_t i) override;

  void remove_constraint(std::size_t i) override;

  std::size_t num_vars() const override;

  std::size_t num_constraints() const override;

  void set_objective_sense(const OptimizationType objsense) override;

  void set_objective(const Objective<double>& objective) override;

  OptimizationType optimization_type() const override;

  Constraint<double> constraint(std::size_t i) const override;

  std::vector<Constraint<double>> constraints() const override;

  Objective<double> objective() const override;

  std::shared_ptr<soplex::SoPlex> soplex(detail::Badge<SoplexSolver>) {
    return soplex_;
  }

 private:
  std::vector<std::size_t> permutation_;
  std::vector<std::size_t> inverse_permutation_;

  std::vector<std::size_t> permutation_vars_;
  std::vector<std::size_t> inverse_permutation_vars_;

  std::shared_ptr<soplex::SoPlex> soplex_;

  OptimizationType sense_ = OptimizationType::Maximize;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_SOPLEX_H