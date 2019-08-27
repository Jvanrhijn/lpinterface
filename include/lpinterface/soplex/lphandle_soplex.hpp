#ifndef LPINTERFACE_LPHANDLE_SOPLEX_H
#define LPINTERFACE_LPHANDLE_SOPLEX_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "soplex.h"

#include "lpinterface/badge.hpp"
#include "lpinterface/lp.hpp"

namespace lpint {

class SoplexSolver;

class LinearProgramHandleSoplex : public ILinearProgramHandle {
 public:
  LinearProgramHandleSoplex(detail::Badge<SoplexSolver>,
                            std::shared_ptr<soplex::SoPlex> soplex)
      : soplex_(soplex) {}

  void add_constraints(std::vector<Constraint<double>>&& constraints) override;

  void remove_constraint(std::size_t i) override;

  std::size_t num_vars() const override;

  std::size_t num_constraints() const override;

  void set_objective_sense(const OptimizationType objsense) override;

  void set_objective(Objective<double>&& objective) override;

  OptimizationType optimization_type() const override;

  std::vector<Constraint<double>> constraints() const override;

  Objective<double> objective() const override;

  std::shared_ptr<soplex::SoPlex> soplex(detail::Badge<SoplexSolver>) {
    return soplex_;
  }

 private:
  using ConstraintIndex = std::size_t;
  using InternalIndex = std::size_t;

  std::vector<InternalIndex> internal_indices_;

  std::shared_ptr<soplex::SoPlex> soplex_;

  OptimizationType sense_ = OptimizationType::Maximize;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_SOPLEX_H