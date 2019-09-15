#ifndef LPINTERFACE_LPHANDLE_SOPLEX_H
#define LPINTERFACE_LPHANDLE_SOPLEX_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <numeric>

#include "soplex.h"

#include "lpinterface/badge.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/detail/util.hpp"

namespace lpint {

class SoplexSolver;

class LinearProgramHandleSoplex : public ILinearProgramHandle {
 public:
  LinearProgramHandleSoplex(detail::Badge<SoplexSolver>,
                            std::shared_ptr<soplex::SoPlex> soplex)
      : soplex_(soplex) 
   {
     std::iota(permutation_.begin(), permutation_.end(), 0);
     inverse_permutation_ = detail::inverse_permutation(permutation_);
   }

  std::vector<Variable> variables() const override;

  void add_variables(const std::vector<Variable>& vars) override;

  void add_variables(std::size_t num_vars) override;

  void add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  void remove_constraint(std::size_t i) override;

  std::size_t num_vars() const override;

  std::size_t num_constraints() const override;

  void set_objective_sense(const OptimizationType objsense) override;

  void set_objective(const Objective<double>& objective) override;

  OptimizationType optimization_type() const override;

  std::vector<Constraint<double>> constraints() const override;

  Objective<double> objective() const override;

  std::shared_ptr<soplex::SoPlex> soplex(detail::Badge<SoplexSolver>) {
    return soplex_;
  }

 private:
  std::vector<std::size_t> permutation_;
  std::vector<std::size_t> inverse_permutation_;

  std::shared_ptr<soplex::SoPlex> soplex_;

  OptimizationType sense_ = OptimizationType::Maximize;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_SOPLEX_H