#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include <memory>

#include "gurobi_c.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver, public FlushRawData<double> {
 public:
  GurobiSolver() = default;
  explicit GurobiSolver(OptimizationType optim_type);
  explicit GurobiSolver(std::unique_ptr<LinearProgramInterface>&& lp);

  ~GurobiSolver();

  // rule of five: should implement/delete these
  GurobiSolver(const GurobiSolver&) = delete;
  GurobiSolver(GurobiSolver&&) noexcept;
  GurobiSolver& operator=(GurobiSolver other) noexcept;

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  void update_program() override;

  Status solve_primal() override;

  Status solve_dual() override;

  Status solution_status() const override;

  const LinearProgramInterface& linear_program() const override;

  LinearProgramInterface& linear_program() override;

  const Solution<double>& get_solution() const override;

  void add_columns(std::vector<double>&& values,
                   std::vector<int>&& start_indices,
                   std::vector<int>&& row_indices, std::vector<Ordering>&& ord,
                   std::vector<double>&& rhs) override;
  void add_rows(std::vector<double>&& values, std::vector<int>&& start_indices,
                std::vector<int>&& col_indices, std::vector<Ordering>&& ord,
                std::vector<double>&& rhs) override;
  void add_variables(std::vector<double>&& objective_values,
                     std::vector<VarType>&& var_types) override;

  static std::vector<char> convert_variable_type(
      const std::vector<VarType>& var_types);
  constexpr static char convert_ordering(const Ordering ord);
  constexpr static Status convert_gurobi_status(int status);

 private:
  void redirect_stdout();
  void restore_stdout();
  int saved_stdout_;
  int new_stdout_;

  constexpr static const char* translate_parameter(const Param param);

  //! The linear program to solve
  std::unique_ptr<LinearProgramInterface> linear_program_;

  //! The gurobi environment object
  GRBenv* gurobi_env_;

  //! The gurobi model object
  GRBmodel* gurobi_model_;

  //! The solution vector
  Solution<double> solution_;

  // copy-and-swap idiom
  friend void swap(GurobiSolver& first, GurobiSolver& second) noexcept {
    using std::swap;
    swap(first.linear_program_, second.linear_program_);
    swap(first.gurobi_env_, second.gurobi_env_);
    swap(first.gurobi_model_, second.gurobi_model_);
    swap(first.solution_, second.solution_);
  }
};

constexpr char GurobiSolver::convert_ordering(const Ordering ord) {
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

constexpr Status GurobiSolver::convert_gurobi_status(int status) {
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

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_GUROBI_H
