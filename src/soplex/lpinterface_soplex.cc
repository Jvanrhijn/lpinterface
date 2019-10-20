#include "lpinterface/soplex/lpinterface_soplex.hpp"
#include "lpinterface/soplex/lputil_soplex.hpp"

namespace lpint {

using namespace soplex;

SoplexSolver::SoplexSolver(OptimizationType optim_type)
    : soplex_(std::make_shared<SoPlex>()), lp_handle_({}, soplex_) {
  lp_handle_.set_objective_sense(optim_type);
  if (!soplex_->setIntParam(
          static_cast<SoPlex::IntParam>(param_dict_.at(Param::Verbosity)), 0)) {
    throw FailedToSetParameterException();
  }
}

SoplexSolver::SoplexSolver()
    : soplex_(std::make_shared<SoPlex>()), lp_handle_({}, soplex_) {}

bool SoplexSolver::parameter_supported(const Param param) const {
  return param_dict_.count(param);
}

void SoplexSolver::set_parameter(const Param param, const int value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  const auto convert_param =
      static_cast<SoPlex::IntParam>(param_dict_.at(param));
  if (!soplex_->setIntParam(convert_param, value)) {
    throw FailedToSetParameterException();
  }
}

void SoplexSolver::set_parameter(const Param param, const double value) {
  if (!parameter_supported(param)) throw UnsupportedParameterException();
  const auto convert_param =
      static_cast<SoPlex::RealParam>(param_dict_.at(param));
  if (!soplex_->setRealParam(convert_param, value)) {
    throw FailedToSetParameterException();
  }
}

Status SoplexSolver::solve() {
  const auto status = translate_status(soplex_->optimize());

  DVector prim(soplex_->numColsReal());
  DVector dual(soplex_->numRowsReal());

  soplex_->getPrimalReal(prim);
  soplex_->getDualReal(dual);

  solution_.primal.resize(static_cast<std::size_t>(prim.dim()));
  std::copy(detail::begin(prim), detail::end(prim), solution_.primal.begin());
  solution_.dual.resize(static_cast<std::size_t>(dual.dim()));
  std::copy(detail::begin(dual), detail::end(dual), solution_.dual.begin());

  solution_.objective_value = soplex_->objValueReal();

  return status;
}

Status SoplexSolver::solution_status() const {
  return translate_status(soplex_->status());
}

const ILinearProgramHandle& SoplexSolver::linear_program() const {
  return lp_handle_;
}

ILinearProgramHandle& SoplexSolver::linear_program() { return lp_handle_; }

const Solution<double>& SoplexSolver::get_solution() const {
  if (solution_status() != Status::Optimal) {
    throw ModelNotSolvedException();
  }
  return solution_;
}

const std::unordered_map<Param, int> SoplexSolver::param_dict_ = {
    {Param::ObjectiveSense, soplex::SoPlex::OBJSENSE},
    {Param::Verbosity, soplex::SoPlex::VERBOSITY},
    {Param::PrimalOrDual, soplex::SoPlex::ALGORITHM},
    {Param::IterationLimit, soplex::SoPlex::ITERLIMIT},
    {Param::Infinity, soplex::SoPlex::INFTY},
    {Param::TimeLimit, soplex::SoPlex::TIMELIMIT},
    {Param::ObjectiveLowerLimit, soplex::SoPlex::OBJLIMIT_LOWER},
    {Param::ObjectiveUpperLimit, soplex::SoPlex::OBJLIMIT_UPPER},
};

const std::unordered_map<SPxSolver::Status, Status> SoplexSolver::status_dict_ =
    {
        {soplex::SPxSolver::Status::NO_RATIOTESTER, Status::NoRatioTester},
        {soplex::SPxSolver::Status::NO_PRICER, Status::NoPricer},
        {soplex::SPxSolver::Status::NO_SOLVER, Status::NoSolver},
        {soplex::SPxSolver::Status::NOT_INIT, Status::NotInitialized},
        {soplex::SPxSolver::Status::ABORT_EXDECOMP, Status::ExitDecomposition},
        {soplex::SPxSolver::Status::ABORT_DECOMP, Status::Decomposition},
        {soplex::SPxSolver::Status::ABORT_CYCLING, Status::Cycling},
        {soplex::SPxSolver::Status::ABORT_TIME, Status::TimeOut},
        {soplex::SPxSolver::Status::ABORT_ITER, Status::IterationLimit},
        {soplex::SPxSolver::Status::ABORT_VALUE, Status::ObjectiveLimit},
        {soplex::SPxSolver::Status::SINGULAR, Status::NumericFailure},
        {soplex::SPxSolver::Status::NO_PROBLEM, Status::NotLoaded},
        {soplex::SPxSolver::Status::REGULAR, Status::Regular},
        {soplex::SPxSolver::Status::RUNNING, Status::InProgress},
        {soplex::SPxSolver::Status::UNKNOWN, Status::NotLoaded},
        {soplex::SPxSolver::Status::OPTIMAL, Status::Optimal},
        {soplex::SPxSolver::Status::UNBOUNDED, Status::Unbounded},
        {soplex::SPxSolver::Status::INFEASIBLE, Status::Infeasible},
        {soplex::SPxSolver::Status::INForUNBD, Status::InfeasibleOrUnbounded},
        {soplex::SPxSolver::Status::OPTIMAL_UNSCALED_VIOLATIONS,
         Status::OptimalUnscaledViolations},
};

}  // namespace lpint