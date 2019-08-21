#include "lpinterface/gurobi/lphandle_gurobi.hpp"

namespace lpint {

std::size_t LinearProgramHandleGurobi::num_vars() const {
    int nvars;
    GRBgetintattr(grb_model_.get(), GRB_INT_ATTR_NUMVARS, &nvars);
    return static_cast<std::size_t>(nvars);
}

void LinearProgramHandleGurobi::set_objective_sense(const OptimizationType objsense) {
    detail::gurobi_function_checked(GRBsetintattr, grb_model_.get(),
        GRB_INT_ATTR_MODELSENSE, objsense == OptimizationType::Maximize? GRB_MAXIMIZE : GRB_MINIMIZE);
}

void LinearProgramHandleGurobi::add_constraints(std::vector<Constraint<double>>&& constraints) {
    for (const auto& constraint : constraints) {
        detail::gurobi_function_checked(GRBaddrangeconstr, grb_model_.get(),
                                            constraint.row.num_nonzero(),
                                            const_cast<int*>(constraint.row.nonzero_indices().data()),
                                            const_cast<double*>(constraint.row.values().data()),
                                            constraint.lower_bound,
                                            constraint.upper_bound, nullptr);
    }
    detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

void LinearProgramHandleGurobi::set_objective(const Objective<double>& objective) {
    const auto nvars = objective.values.size();
    detail::gurobi_function_checked(GRBaddvars, grb_model_.get(), 
                                  nvars, 0, nullptr, nullptr, nullptr,
                                  const_cast<double*>(objective.values.data()), nullptr, nullptr, 
                                  convert_variable_type(objective.variable_types).data(), nullptr);
    detail::gurobi_function_checked(GRBupdatemodel, grb_model_.get());
}

OptimizationType LinearProgramHandleGurobi::optimization_type() const {
    int sense;
    detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(), GRB_INT_ATTR_MODELSENSE, &sense);
    return sense == GRB_MAXIMIZE ? OptimizationType::Maximize : OptimizationType::Minimize;
}

std::vector<Constraint<double>> LinearProgramHandleGurobi::constraints() const {
    // retrieve number of constraints
    int nconstr;
    detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(), GRB_INT_ATTR_NUMCONSTRS, &nconstr);
    int nvars;
    detail::gurobi_function_checked(GRBgetintattr, grb_model_.get(), GRB_INT_ATTR_NUMVARS, &nvars);
    std::cout << "Nvars: " << nvars << std::endl;
    std::vector<Constraint<double>> constraints;
    for (int i = 0; i < nconstr; i++) {
        // retrieve size of data
        int nnz;
        detail::gurobi_function_checked(GRBgetconstrs, grb_model_.get(), &nnz, nullptr, nullptr, nullptr, i, 1);
        // allocate data
        Row<double> row(static_cast<std::size_t>(nnz));
        int cbeg;
        detail::gurobi_function_checked(GRBgetconstrs, grb_model_.get(), &nnz, &cbeg, row.nonzero_indices().data(), row.values().data(), i, 1);
        // range constraints add extra variables to keep track of range,
        // so to get the bounds of this constraint we check the corresponding
        // range variable.
        double lb, ub;
        detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(), GRB_DBL_ATTR_LB, nvars + i - 1, &lb);
        detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(), GRB_DBL_ATTR_UB, nvars + i - 1, &ub);
        constraints.emplace_back(std::move(row), lb, ub);
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
        detail::gurobi_function_checked(GRBgetdblattrelement, grb_model_.get(), GRB_DBL_ATTR_OBJ, i, &obj);
        detail::gurobi_function_checked(GRBgetcharattrelement, grb_model_.get(), GRB_CHAR_ATTR_VTYPE, i, &vtype);
        values.push_back(obj);
        var_types.push_back([&]() {
            switch (vtype) {
                case 'C': return VarType::Real;
                case 'B': return VarType::Binary;
                case 'I': return VarType::Integer;
                case 'S': return VarType::SemiReal;
                case 'N': return VarType::SemiInteger;
                default: throw UnsupportedVariableTypeException();
            }
        }());
    }
    return Objective<double>(std::move(values), std::move(var_types));
}

} // namespace lpint