#include "lpinterface/gurobi/lphandle_gurobi.hpp"

namespace lpint {

std::size_t LinearProgramHandleGurobi::num_vars() const {
    int nvars;
    GRBgetintattr(grb_model_.get(), GRB_INT_ATTR_NUMVARS, &nvars);
    return static_cast<std::size_t>(nvars);
}

void LinearProgramHandleGurobi::set_objective_sense(const OptimizationType objsense) {
    if (auto error = GRBsetintattr(grb_model_.get(), GRB_INT_ATTR_MODELSENSE, 
        objsense == OptimizationType::Maximize? GRB_MAXIMIZE : GRB_MINIMIZE)) {
        throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
    } 
}

void LinearProgramHandleGurobi::add_constraints(std::vector<Constraint<double>>&& constraints) {
    for (const auto& constraint : constraints) {
        const auto error = GRBaddrangeconstr(grb_model_.get(), 
                                             constraint.row.num_nonzero(),
                                             const_cast<int*>(constraint.row.nonzero_indices().data()),
                                             const_cast<double*>(constraint.row.values().data()),
                                             constraint.lower_bound,
                                             constraint.upper_bound, nullptr);
        if (error) {
            throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
        }
    }
}

void LinearProgramHandleGurobi::set_objective(const Objective<double>& objective) {
    const auto nvars = objective.values.size();
    const auto error = GRBaddvars(grb_model_.get(), nvars, 0, nullptr, nullptr, nullptr,
                                  const_cast<double*>(objective.values.data()), nullptr, nullptr, 
                                  convert_variable_type(objective.variable_types).data(), nullptr);
    if (error) {
        throw GurobiException(error);
    }
    GRBupdatemodel(grb_model_.get());
}

OptimizationType LinearProgramHandleGurobi::optimization_type() const {
    int sense;
    GRBgetintattr(grb_model_.get(), GRB_INT_ATTR_MODELSENSE, &sense);
    return sense == GRB_MAXIMIZE ? OptimizationType::Maximize : OptimizationType::Minimize;
}

std::vector<Constraint<double>> LinearProgramHandleGurobi::constraints() const {
    throw NotImplementedError();
    // retrieve number of constraints
    int nconstr;
    GRBgetintattr(grb_model_.get(), GRB_INT_ATTR_NUMCONSTRS, &nconstr);
    std::vector<Constraint<double>> constraints;
    for (int i = 0; i < nconstr; i++) {
        int nnz;
        // retrieve size of data
        if (auto error = GRBgetconstrs(grb_model_.get(), &nnz, nullptr, nullptr, nullptr, i, 1)) {
            throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
        }
        // allocate data
        Row<double> row(static_cast<std::size_t>(nnz));
        int cbeg;
        if (auto error = GRBgetconstrs(grb_model_.get(), &nnz, &cbeg, row.nonzero_indices().data(), row.values().data(), i, 1)) {
            throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
        }
        double rhs;
        char sense;
        GRBgetdblattrelement(grb_model_.get(), GRB_DBL_ATTR_RHS, i, &rhs);
        GRBgetcharattrelement(grb_model_.get(), GRB_CHAR_ATTR_SENSE, i, &sense);
    }
}

Objective<double> LinearProgramHandleGurobi::objective() const {
    const auto nvars = num_vars();
    std::vector<double> values;
    std::vector<VarType> var_types;
    for (std::size_t i = 0; i < nvars; i++) {
        double obj;
        char vtype;
        if (const auto error = GRBgetdblattrelement(grb_model_.get(), GRB_DBL_ATTR_OBJ, i, &obj)) {
            throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
        }
        if (const auto error = GRBgetcharattrelement(grb_model_.get(), GRB_CHAR_ATTR_VTYPE, i, &vtype)) {
            throw GurobiException(error, GRBgeterrormsg(grb_env_.get()));
        }
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