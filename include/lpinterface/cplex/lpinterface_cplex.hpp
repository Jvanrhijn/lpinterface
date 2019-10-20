#ifndef LPINTERFACE_LPINTERFACE_CPLEX_H
#define LPINTERFACE_LPINTERFACE_CPLEX_H

#include "lpinterface/badge.hpp"
#include "lpinterface/lpinterface.hpp"
#include "lpinterface/cplex/lphandle_cplex.hpp"

namespace lpint {

class CplexSolver : public LinearProgramSolver {
  bool parameter_supported(const Param param) const override;

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  Status solve() override;

  Status solution_status() const override;

  const ILinearProgramHandle& linear_program() const override;

  ILinearProgramHandle& linear_program() override;

  const Solution<double>& get_solution() const override;
};


} // namespace lpint

#endif // LPINTERFACE_LPINTERFACE_CPLEX_H