#include "lpinterface/gurobi/lphandle_gurobi.hpp"

namespace lpint {

std::size_t LinearProgramHandleGurobi::num_vars() const { return num_vars_; }

std::size_t LinearProgramHandleGurobi::num_constraints() const {
  return num_constraints_;
}

void LinearProgramHandleGurobi::set_objective_sense(
    const OptimizationType objsense) {
  detail::gurobi_function_checked(
      GRBsetintattr, grb_model_.get(), GRB_INT_ATTR_MODELSENSE,
      objsense == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

Variable LinearProgramHandleGurobi::variable(std::size_t i) const {
  double lb, ub;
  detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(),
                                  GRB_DBL_ATTR_LB, i, &lb);
  detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(),
                                  GRB_DBL_ATTR_UB, i, &ub);
  return Variable(lb, ub);
}

std::vector<Variable> LinearProgramHandleGurobi::variables() const {
  std::vector<Variable> vars;
  for (std::size_t i = 0; i < num_vars_; i++) {
    vars.emplace_back(variable(i));
  }
  return vars;
}

void LinearProgramHandleGurobi::add_variables(
    const std::vector<Variable>& vars) {
  num_vars_ += vars.size();
  for (const auto& var : vars) {
    detail::gurobi_function_checked(GRBaddvar, grb_model_.get(), 0, nullptr,
                                    nullptr, 0.0, var.lower(), var.upper(),
                                    GRB_CONTINUOUS, nullptr);
  }
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

void LinearProgramHandleGurobi::add_variables(const std::size_t num_vars) {
  add_variables(std::vector<Variable>(num_vars));
}

void LinearProgramHandleGurobi::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  for (const auto& constraint : constraints) {
    detail::gurobi_function_checked(
        GRBaddrangeconstr, grb_model_.get(), constraint.row.num_nonzero(),
        const_cast<Constraint<double>&>(constraint)
            .row.nonzero_indices()
            .data(),
        const_cast<Constraint<double>&>(constraint).row.values().data(),
        constraint.lower_bound, constraint.upper_bound, nullptr);
    // keep track of these internally since
    // gurobi mixes them up with the range variables
    lower_bounds.push_back(constraint.lower_bound);
    upper_bounds.push_back(constraint.upper_bound);
    num_constraints_++;
  }
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

void LinearProgramHandleGurobi::remove_variable(const std::size_t i) {
  auto to_del = static_cast<int>(i);
  detail::gurobi_function_checked(GRBdelvars, grb_model_.get(), 1, &to_del);
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
  num_vars_--;
}

void LinearProgramHandleGurobi::remove_constraint(std::size_t i) {
  int to_del = static_cast<int>(i);
  detail::gurobi_function_checked(GRBdelconstrs, grb_model_.get(), 1, &to_del);
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
  num_constraints_--;
}

void LinearProgramHandleGurobi::set_objective(
    const Objective<double>& objective) {
  if (num_vars_ != objective.values.size()) {
    throw MismatchedDimensionsException();
  }
  detail::gurobi_function_checked(
      GRBsetdblattrarray, grb_model_.get(), GRB_DBL_ATTR_OBJ, 0, num_vars_,
      const_cast<Objective<double>&>(objective).values.data());
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

OptimizationType LinearProgramHandleGurobi::optimization_type() const {
  int sense;
  detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(),
                                  GRB_INT_ATTR_MODELSENSE, &sense);
  return sense == GRB_MAXIMIZE ? OptimizationType::Maximize
                               : OptimizationType::Minimize;
}

Constraint<double> LinearProgramHandleGurobi::constraint(std::size_t i) const {
  int nnz;
  detail::gurobi_function_checked(GRBgetconstrs, grb_model_.get(), &nnz,
                                  nullptr, nullptr, nullptr, i, 1);
  // allocate data
  std::vector<double> values(static_cast<std::size_t>(nnz));
  std::vector<int> indices(static_cast<std::size_t>(nnz));
  int cbeg;
  detail::gurobi_function_checked(GRBgetconstrs, grb_model_.get(), &nnz,
                                  &cbeg, indices.data(), values.data(), i, 1);

  Row<double> row(std::vector<double>(values.begin(), values.end() - 1),
                  std::vector<int>(indices.begin(), indices.end() - 1));
  return Constraint<double>(std::move(row), lower_bounds[i], upper_bounds[i]);
}

std::vector<Constraint<double>> LinearProgramHandleGurobi::constraints() const {
  // retrieve number of constraints
  std::vector<Constraint<double>> constraints;
  for (std::size_t i = 0; i < num_constraints_; i++) {
    constraints.emplace_back(constraint(i));
  }
  return constraints;
}

Objective<double> LinearProgramHandleGurobi::objective() const {
  const auto nvars = num_vars();
  std::vector<double> values;
  for (std::size_t i = 0; i < nvars; i++) {
    double obj;
    detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(),
                                    GRB_DBL_ATTR_OBJ, i, &obj);
    values.push_back(obj);
  }
  return Objective<double>(std::move(values));
}

std::shared_ptr<GRBmodel> LinearProgramHandleGurobi::gurobi_model(
    detail::Badge<GurobiSolver>) const {
  return grb_model_;
}

std::shared_ptr<GRBenv> LinearProgramHandleGurobi::gurobi_env(
    detail::Badge<GurobiSolver>) const {
  return grb_env_;
}

}  // namespace lpint