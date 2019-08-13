#include "lpinterface/lp_impl.hpp"

namespace lpint {

LinearProgram::LinearProgram(const OptimizationType opt_type)
    : opt_type_(opt_type) {}

std::size_t LinearProgram::num_vars() const { return objective_.values.size(); }

const std::vector<Constraint<double>>& LinearProgram::constraints() const {
  return constraints_;
}

std::vector<Constraint<double>>& LinearProgram::constraints() {
  return constraints_;
}

void LinearProgram::add_constraints(
    std::vector<Constraint<double>>&& constraints) {
  constraints_ = std::move(constraints);
}

OptimizationType LinearProgram::optimization_type() const { return opt_type_; }

void LinearProgram::set_objective(const Objective<double>& objective) {
  objective_ = objective;
}

const Objective<double>& LinearProgram::objective() const { return objective_; }

Objective<double>& LinearProgram::objective() { return objective_; }

bool LinearProgram::is_initialized() const {
  return !constraints_.empty() && !objective_.values.empty();
}

}  // namespace lpint
