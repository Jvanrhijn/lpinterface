#ifndef LPINTERFACE_LPHANDLE_GUROBI_H
#define LPINTERFACE_LPHANDLE_GUROBI_H

#include <memory>
#include <vector>
#include <cstddef>

#include "gurobi_c.h"

#include "lpinterface/lp.hpp"

namespace lpint {

class GrbData {
 public:
  GrbData() {
    GRBloadenv(&gurobi_env, "");
    GRBnewmodel(gurobi_env, &gurobi_model, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr);
  }
  
  ~GrbData() {
    GRBfreeenv(gurobi_env);
    GRBfreemodel(gurobi_model);
  }

 private:
  GRBenv *gurobi_env;
  GRBmodel *gurobi_model;
};

class LinearProgramHandleGurobi : public ILinearProgramHandle {
 public:
  LinearProgramHandleGurobi() = default;
  LinearProgramHandleGurobi(std::shared_ptr<GRBmodel> grbmodel, std::shared_ptr<GRBenv> grbenv)
    : grb_model_(grbmodel), grb_env_(grbenv) {}

  std::size_t num_vars() const override;

  void set_objective_sense(const OptimizationType objsense) override;

  void add_constraints(std::vector<Constraint<double>>&& constraints) override;

  OptimizationType optimization_type() const override;

  void set_objective(const Objective<double>& objective) override;

  std::vector<Constraint<double>> constraints() const override;

  Objective<double> objective() const override;

  inline static std::vector<char> convert_variable_type(const std::vector<VarType>& var_types);

 private:
  std::shared_ptr<GRBmodel> grb_model_;
  std::shared_ptr<GRBenv> grb_env_;


};

std::vector<char> LinearProgramHandleGurobi::convert_variable_type(
    const std::vector<VarType>& var_types) {
  const auto num_vars = var_types.size();
  std::vector<char> value_type(num_vars);
  for (std::size_t i = 0; i < num_vars; i++) {
    char vtype;
    switch (var_types[i]) {
      case VarType::Binary:
        vtype = GRB_BINARY;
        break;
      case VarType::Integer:
        vtype = GRB_INTEGER;
        break;
      case VarType::Real:
        vtype = GRB_CONTINUOUS;
        break;
      case VarType::SemiReal:
        vtype = GRB_SEMICONT;
        break;
      case VarType::SemiInteger:
        vtype = GRB_SEMIINT;
        break;
      default:
        throw UnsupportedVariableTypeException();
    }
    value_type[i] = vtype;
  }
  return value_type;
}

} // namespace lpint

#endif // LPINTERFACE_LPHANDLE_GUROBI_H