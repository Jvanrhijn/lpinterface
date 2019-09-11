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
  // static std::size_t transform_index(std::size_t removed, std::size_t i,
  // std::size_t length) {
  //  if (i < removed) {
  //    return i;
  //  } else if (i == length - 2) {
  //    return removed;
  //  } else {
  //    return i + 1;
  //  }
  //}

  using InternalIndex = std::size_t;

  // std::vector<InternalIndex> internal_indices_;

  std::shared_ptr<soplex::SoPlex> soplex_;

  OptimizationType sense_ = OptimizationType::Maximize;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_SOPLEX_H