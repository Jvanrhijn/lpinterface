#ifndef LPINTERFACE_LPINTERFACE_SOPLEX_H
#define LPINTERFACE_LPINTERFACE_SOPLEX_H

#include <algorithm>
#include <memory>

#include "soplex.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"

namespace lpint {

class SoplexSolver : public LinearProgramSolver, public FlushRawData<double> {
 public:
  SoplexSolver() = default;
  explicit SoplexSolver(OptimizationType optim_type);
  explicit SoplexSolver(std::unique_ptr<LinearProgramInterface>&& lp);

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

  #if __cplusplus >= 201402L
  constexpr
  #endif
  static Status translate_status(
      const soplex::SPxSolver::Status status);

 private:
  soplex::SoPlex soplex_;

  std::unique_ptr<LinearProgramInterface> linear_program_;

  Solution<double> solution_;

  #if __cplusplus >= 201402L
  constexpr
  #endif
  static soplex::SoPlex::IntParam translate_int_parameter(
      const Param param);

  #if __cplusplus >= 201402L
  constexpr
  #endif
  static soplex::SoPlex::RealParam translate_real_parameter(
      const Param param);
};

#if __cplusplus >= 201402L
constexpr
#endif
inline Status SoplexSolver::translate_status(
    const soplex::SPxSolver::Status status) {
  switch (status) {
    case soplex::SPxSolver::Status::ERROR:
      throw SoplexException();
    case soplex::SPxSolver::Status::NO_RATIOTESTER:
      return Status::NoRatioTester;
    case soplex::SPxSolver::Status::NO_PRICER:
      return Status::NoPricer;
    case soplex::SPxSolver::Status::NO_SOLVER:
      return Status::NoSolver;
    case soplex::SPxSolver::Status::NOT_INIT:
      return Status::NotInitialized;
    case soplex::SPxSolver::Status::ABORT_EXDECOMP:
      return Status::ExitDecomposition;
    case soplex::SPxSolver::Status::ABORT_DECOMP:
      return Status::Decomposition;
    case soplex::SPxSolver::Status::ABORT_CYCLING:
      return Status::Cycling;
    case soplex::SPxSolver::Status::ABORT_TIME:
      return Status::TimeOut;
    case soplex::SPxSolver::Status::ABORT_ITER:
      return Status::IterationLimit;
    case soplex::SPxSolver::Status::ABORT_VALUE:
      return Status::ObjectiveLimit;
    case soplex::SPxSolver::Status::SINGULAR:
      return Status::NumericFailure;
    case soplex::SPxSolver::Status::NO_PROBLEM:
      return Status::NotLoaded;
    case soplex::SPxSolver::Status::REGULAR:
      return Status::Regular;
    case soplex::SPxSolver::Status::RUNNING:
      return Status::InProgress;
    case soplex::SPxSolver::Status::UNKNOWN:
      return Status::NotLoaded;
    case soplex::SPxSolver::Status::OPTIMAL:
      return Status::Optimal;
    case soplex::SPxSolver::Status::UNBOUNDED:
      return Status::Unbounded;
    case soplex::SPxSolver::Status::INFEASIBLE:
      return Status::Infeasible;
    case soplex::SPxSolver::Status::INForUNBD:
      return Status::InfeasibleOrUnbounded;
    case soplex::SPxSolver::Status::OPTIMAL_UNSCALED_VIOLATIONS:
      return Status::OptimalUnscaledViolations;
    default:
      throw UnknownStatusException(status);
  }
}

// TODO: extend
#if __cplusplus >= 201402L
constexpr
#endif
inline soplex::SoPlex::IntParam SoplexSolver::translate_int_parameter(
    const Param param) {
  using namespace soplex;
  switch (param) {
    case (Param::ObjectiveSense):
      return SoPlex::OBJSENSE;
    case (Param::Verbosity):
      return SoPlex::VERBOSITY;
    case (Param::PrimalOrDual):
      return SoPlex::ALGORITHM;
    case (Param::IterationLimit):
      return SoPlex::ITERLIMIT;
    default:
      throw UnsupportedParameterException();
  }
}

// TODO: extend
#if __cplusplus >= 201402L
constexpr
#endif
inline soplex::SoPlex::RealParam SoplexSolver::translate_real_parameter(
    const Param param) {
  using namespace soplex;
  switch (param) {
    case (Param::Infinity):
      return SoPlex::INFTY;
    case (Param::TimeLimit):
      return SoPlex::TIMELIMIT;
    case (Param::ObjectiveLowerLimit):
      return SoPlex::OBJLIMIT_LOWER;
    case (Param::ObjectiveUpperLimit):
      return SoPlex::OBJLIMIT_UPPER;
    default:
      throw UnsupportedParameterException();
  }
}

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_SOPLEX_H