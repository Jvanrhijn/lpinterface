#ifndef LPINTERFACE_LPHANDLE_GUROBI_H
#define LPINTERFACE_LPHANDLE_GUROBI_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

#include "gurobi_c.h"

#include "lpinterface/badge.hpp"
#include "lpinterface/gurobi/lputil_gurobi.hpp"
#include "lpinterface/lp.hpp"

namespace lpint {

class GurobiSolver;

class LinearProgramHandleGurobi : public ILinearProgramHandle {
 public:
  LinearProgramHandleGurobi(detail::Badge<GurobiSolver>,
                            std::shared_ptr<GRBmodel> grbmodel,
                            std::shared_ptr<GRBenv> grbenv)
      : grb_env_(grbenv), grb_model_(grbmodel) {}

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

  std::shared_ptr<GRBmodel> gurobi_model(detail::Badge<GurobiSolver>) const;
  std::shared_ptr<GRBenv> gurobi_env(detail::Badge<GurobiSolver>) const;

  // TODO: find a better way to do this!!!
  void set_num_vars(detail::Badge<GurobiSolver>, std::size_t nvars) {
    num_vars_ = nvars;
  }

 private:
  std::shared_ptr<GRBenv> grb_env_;
  std::shared_ptr<GRBmodel> grb_model_;

  std::vector<double> upper_bounds;
  std::vector<double> lower_bounds;

  std::size_t num_vars_ = 0;
  std::size_t num_constraints_ = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPHANDLE_GUROBI_H