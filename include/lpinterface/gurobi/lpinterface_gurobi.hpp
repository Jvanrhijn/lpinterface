#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include <memory>

#include "gurobi_c.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/gurobi/lphandle_gurobi.hpp"
#include "lpinterface/gurobi/lputil_gurobi.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver, public FlushRawData<double> {
 public:
  GurobiSolver() : gurobi_env_(nullptr), gurobi_model_(nullptr), lp_handle_() {}
  explicit GurobiSolver(OptimizationType optim_type);
  explicit GurobiSolver(LinearProgramHandleGurobi&& lp_handle)
      : gurobi_env_(lp_handle.gurobi_env(detail::Badge<GurobiSolver>{})),
        gurobi_model_(lp_handle.gurobi_model(detail::Badge<GurobiSolver>{})),
        lp_handle_(std::move(lp_handle)) {}

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  void update_program() override;

  Status solve_primal() override;

  Status solve_dual() override;

  Status solution_status() const override;

  const ILinearProgramHandle& linear_program() const override;

  ILinearProgramHandle& linear_program() override;

  const Solution<double>& get_solution() const override;

  void add_columns(std::vector<double>&& values,
                   std::vector<int>&& start_indices,
                   std::vector<int>&& row_indices, std::vector<double>&& lb,
                   std::vector<double>&& ub) override;
  void add_rows(std::vector<double>&& values, std::vector<int>&& start_indices,
                std::vector<int>&& col_indices, std::vector<double>&& lb,
                std::vector<double>&& ub) override;
  void add_variables(std::vector<double>&& objective_values,
                     std::vector<VarType>&& var_types) override;

  static std::vector<char> convert_variable_type(
      const std::vector<VarType>& var_types);

#if __cplusplus >= 201402L
  constexpr
#endif
      static char
      convert_ordering(const Ordering ord);

#if __cplusplus >= 201402L
  constexpr
#endif
      static Status
      convert_gurobi_status(int status);

 private:
#if __cplusplus >= 201402L
  constexpr
#endif
      static const char*
      translate_parameter(const Param param);

  //! The gurobi environment object
  std::shared_ptr<GRBenv> gurobi_env_;

  //! The gurobi model object
  std::shared_ptr<GRBmodel> gurobi_model_;

  //! The linear program to solve
  LinearProgramHandleGurobi lp_handle_;

  //! The solution vector
  Solution<double> solution_;
};

#if __cplusplus >= 201402L
constexpr
#endif
    inline char
    convert_ordering(const Ordering ord) {
  switch (ord) {
    case Ordering::LEQ:
      return GRB_LESS_EQUAL;
    case Ordering::GEQ:
      return GRB_GREATER_EQUAL;
    case Ordering::EQ:
      return GRB_EQUAL;
    default:
      throw UnsupportedConstraintException();
  }
}

#if __cplusplus >= 201402L
constexpr
#endif
    inline Status
    GurobiSolver::convert_gurobi_status(int status) {
  switch (status) {
    case GRB_LOADED:
      return Status::NoInformation;
    case GRB_OPTIMAL:
      return Status::Optimal;
    case GRB_INFEASIBLE:
      return Status::Infeasible;
    case GRB_INF_OR_UNBD:
      return Status::InfeasibleOrUnbounded;
    case GRB_UNBOUNDED:
      return Status::Unbounded;
    case GRB_CUTOFF:
      return Status::Cutoff;
    case GRB_ITERATION_LIMIT:
      return Status::IterationLimit;
    case GRB_NODE_LIMIT:
      return Status::NodeLimit;
    case GRB_TIME_LIMIT:
      return Status::TimeOut;
    case GRB_SOLUTION_LIMIT:
      return Status::SolutionLimit;
    case GRB_INTERRUPTED:
      return Status::Interrupted;
    case GRB_NUMERIC:
      return Status::NumericFailure;
    case GRB_SUBOPTIMAL:
      return Status::SuboptimalSolution;
    case GRB_INPROGRESS:
      return Status::InProgress;
    case GRB_USER_OBJ_LIMIT:
      return Status::UserObjectiveLimit;
    default:
      throw UnknownStatusException(status);
  }
}

// TODO: extend
#if __cplusplus >= 201402L
constexpr
#endif
    inline const char*
    GurobiSolver::translate_parameter(const Param param) {
  switch (param) {
    case (Param::Verbosity):
      return "outputflag";
    case (Param::Threads):
      return "threads";
    case (Param::Cutoff):
      return "Cutoff";
    case (Param::TimeLimit):
      return "TimeLimit";
    case (Param::IterationLimit):
      return "IterationLimit";
    default:
      throw UnsupportedParameterException();
  }
}

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_GUROBI_H
