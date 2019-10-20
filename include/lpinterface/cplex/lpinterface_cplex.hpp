#ifndef LPINTERFACE_LPINTERFACE_CPLEX_H
#define LPINTERFACE_LPINTERFACE_CPLEX_H

#include "lpinterface/badge.hpp"
#include "lpinterface/lpinterface.hpp"
#include "lpinterface/cplex/lphandle_cplex.hpp"

// Magic tricks to have CPLEX behave well:
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// End magic tricks

#include <memory> 

namespace lpint {

class CplexSolver : public LinearProgramSolver {
 public:

  explicit CplexSolver(OptimizationType optim_type);

  CplexSolver();

  bool parameter_supported(const Param param) const override;

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  Status solve() override;

  Status solution_status() const override;

  const ILinearProgramHandle& linear_program() const override;

  ILinearProgramHandle& linear_program() override;

  const Solution<double>& get_solution() const override;
 
 private:
  IloEnv env_;
  std::shared_ptr<IloCplex> cplex_;
  LinearProgramHandleCplex lp_handle_;

  static const std::unordered_map<Param, IloCplex::IntParam> int_param_dict_;

};


} // namespace lpint

#endif // LPINTERFACE_LPINTERFACE_CPLEX_H