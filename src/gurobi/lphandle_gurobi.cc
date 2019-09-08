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

void LinearProgramHandleGurobi::remove_constraint(std::size_t i) {
  int to_del = static_cast<int>(i);
  detail::gurobi_function_checked(GRBdelconstrs, grb_model_.get(), 1, &to_del);
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

void LinearProgramHandleGurobi::set_objective(
    const Objective<double>& objective) {
  num_vars_ = objective.values.size();
  detail::gurobi_function_checked(
      GRBaddvars, grb_model_.get(), num_vars_, 0, nullptr, nullptr, nullptr,
      const_cast<Objective<double>&>(objective).values.data(), nullptr, nullptr,
      convert_variable_type(objective.variable_types).data(), nullptr);
  detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

OptimizationType LinearProgramHandleGurobi::optimization_type() const {
  int sense;
  detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(),
                                  GRB_INT_ATTR_MODELSENSE, &sense);
  return sense == GRB_MAXIMIZE ? OptimizationType::Maximize
                               : OptimizationType::Minimize;
}

std::vector<Constraint<double>> LinearProgramHandleGurobi::constraints() const {
  // retrieve number of constraints
  int nconstr;
  detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(),
                                  GRB_INT_ATTR_NUMCONSTRS, &nconstr);
  // retrieve number of variables in gurobi
  int nvars;
  detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(),
                                  GRB_INT_ATTR_NUMVARS, &nvars);

  std::vector<Constraint<double>> constraints;
  for (std::size_t i = 0; i < nconstr; i++) {
    // retrieve size of data
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
    constraints.emplace_back(std::move(row), lower_bounds[i], upper_bounds[i]);
  }

  return constraints;
}

Objective<double> LinearProgramHandleGurobi::objective() const {
  const auto nvars = num_vars();
  std::vector<double> values;
  std::vector<VarType> var_types;
  for (std::size_t i = 0; i < nvars; i++) {
    double obj;
    char vtype;
    detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(),
                                    GRB_DBL_ATTR_OBJ, i, &obj);
    detail::gurobi_function_checked(GRBgetcharattrelement, grb_model_.get(),
                                    GRB_CHAR_ATTR_VTYPE, i, &vtype);
    values.push_back(obj);
    var_types.push_back([&]() {
      switch (vtype) {
        case 'C':
          return VarType::Real;
        case 'B':
          return VarType::Binary;
        case 'I':
          return VarType::Integer;
        case 'S':
          return VarType::SemiReal;
        case 'N':
          return VarType::SemiInteger;
        default:
          throw UnsupportedVariableTypeException();
      }
    }());
  }
  return Objective<double>(std::move(values), std::move(var_types));
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