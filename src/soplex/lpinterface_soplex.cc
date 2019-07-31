#include "lpinterface/soplex/lpinterface_soplex.hpp"

namespace lpint {

using namespace soplex;

SoplexSolver::SoplexSolver(OptimizationType optim_type) {
  if (!soplex_.setIntParam(SoPlex::OBJSENSE,
                           optim_type == OptimizationType::Maximize
                               ? SoPlex::OBJSENSE_MAXIMIZE
                               : SoPlex::OBJSENSE_MINIMIZE)) {
    throw FailedToSetParameterException();
  }
}

SoplexSolver::SoplexSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  if (!soplex_.setIntParam(SoPlex::OBJSENSE,
                           lp->optimization_type() == OptimizationType::Maximize
                               ? SoPlex::OBJSENSE_MAXIMIZE
                               : SoPlex::OBJSENSE_MINIMIZE)) {
    throw FailedToSetParameterException();
  }
  // TODO: figure out whether this is indeed what SoPlex::REPRESENTATION means
  // soplex_.setIntParam(SoPlex::REPRESENTATION, lp->matrix().type() ==
  // SparseMatrixType::RowWise?
  //  SoPlex::REPRESENTATION_ROW : SoPlex::REPRESENTATION_COLUMN);
}

void SoplexSolver::set_parameter(const Param param, const int value) {
  if (!soplex_.setIntParam(translate_int_parameter(param), value)) {
    throw FailedToSetParameterException();
  }
}

void SoplexSolver::set_parameter(const Param param, const double value) {
  if (!soplex_.setRealParam(translate_real_parameter(param), value)) {
    throw FailedToSetParameterException();
  }
}

void SoplexSolver::update_program() { throw NotImplementedError(); }

Status SoplexSolver::solve_primal() { throw NotImplementedError(); }

Status SoplexSolver::solve_dual() { throw NotImplementedError(); }

Status SoplexSolver::solution_status() const { throw NotImplementedError(); }

const LinearProgramInterface& SoplexSolver::linear_program() const {
  return *linear_program_;
}

LinearProgramInterface& SoplexSolver::linear_program() {
  return *linear_program_;
}

const Solution<double>& SoplexSolver::get_solution() const {
  throw NotImplementedError();
}

constexpr SoPlex::IntParam SoplexSolver::translate_int_parameter(
    const Param param) {
  switch (param) {
    case (Param::ObjectiveSense):
      return SoPlex::OBJSENSE;
    case (Param::Verbosity):
      return SoPlex::VERBOSITY;
    case (Param::PrimalOrDual):
      return SoPlex::ALGORITHM;
    default:
      throw UnsupportedParameterException();
  }
}

constexpr SoPlex::RealParam SoplexSolver::translate_real_parameter(
    const Param param) {
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