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
  set_parameter(Param::Verbosity, 0);
}

SoplexSolver::SoplexSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  if (!soplex_.setIntParam(SoPlex::OBJSENSE,
                           lp->optimization_type() == OptimizationType::Maximize
                               ? SoPlex::OBJSENSE_MAXIMIZE
                               : SoPlex::OBJSENSE_MINIMIZE)) {
    throw FailedToSetParameterException();
  }
  set_parameter(Param::Verbosity, 0);
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

void SoplexSolver::update_program() { 
    if (linear_program_->matrix().type() != SparseMatrixType::RowWise) {
        throw NotImplementedError();
    }
    const auto objective = linear_program_->objective();
    const auto constraints = linear_program_->constraints();

    // add variables to LP
    DSVector dummycol(0);
    for (const auto& coefficient : objective.values) {
      soplex_.addColReal(LPCol(coefficient, dummycol, infinity, 0.0));
    }

    // add constraints to LP
    std::size_t i = 0;
    for (const auto& row : linear_program_->matrix()) {
        DSVector ds_row(row.num_nonzero());
        // TODO: fix this so we don't have to copy each time
        std::vector<int> why(row.nonzero_indices().begin(), row.nonzero_indices().end());
        ds_row.add(row.num_nonzero(), why.data(), row.values().data());
        // determine constraint
        if (constraints[i].ordering == Ordering::LEQ) {
            soplex_.addRowReal(LPRow(0, ds_row, constraints[i].value));
        } else if (constraints[i].ordering == Ordering::GEQ) {
            soplex_.addRowReal(LPRow(constraints[i].value, ds_row, infinity));
        } else {
            throw UnsupportedConstraintException();
        }

        i++;
    }

 }

Status SoplexSolver::solve_primal() { 
    auto status = translate_status(soplex_.optimize());

    DVector prim(soplex_.numColsReal());
    DVector dual(soplex_.numRowsReal());

    soplex_.getPrimalReal(prim);
    soplex_.getDualReal(dual);

    solution_.primal = std::vector<double>(prim.get_ptr(), prim.get_ptr() + prim.dim());
    solution_.dual = std::vector<double>(dual.get_ptr(), dual.get_ptr() + dual.dim());
    solution_.objective_value = soplex_.objValueReal();

    return status;
}

Status SoplexSolver::solve_dual() { 
    return solve_primal(); 
}

Status SoplexSolver::solution_status() const { 
    return translate_status(soplex_.status());
}

const LinearProgramInterface& SoplexSolver::linear_program() const {
  return *linear_program_;
}

LinearProgramInterface& SoplexSolver::linear_program() {
  return *linear_program_;
}

const Solution<double>& SoplexSolver::get_solution() const { return solution_; }

// TODO: extend
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

// TODO: extend
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

constexpr Status SoplexSolver::translate_status(const SPxSolver::Status status) {
    switch (status) {
        case SPxSolver::Status::ERROR: 
            throw SoplexException();
        case SPxSolver::Status::NO_RATIOTESTER:
            return Status::NoRatioTester;
        case SPxSolver::Status::NO_PRICER:
            return Status::NoPricer;
        case SPxSolver::Status::NO_SOLVER:
            return Status::NoSolver;
        case SPxSolver::Status::NOT_INIT:
            return Status::NotInitialized;
        case SPxSolver::Status::ABORT_EXDECOMP:
            return Status::ExitDecomposition;
        case SPxSolver::Status::ABORT_DECOMP:
            return Status::Decomposition;
        case SPxSolver::Status::ABORT_CYCLING:
            return Status::Cycling;
        case SPxSolver::Status::ABORT_TIME:
            return Status::TimeOut;
        case SPxSolver::Status::ABORT_ITER:
            return Status::IterationLimit;
        case SPxSolver::Status::ABORT_VALUE:
            return Status::ObjectiveLimit;
        case SPxSolver::Status::SINGULAR:
            return Status::NumericFailure;
        case SPxSolver::Status::NO_PROBLEM:
            return Status::NotLoaded;
        case SPxSolver::Status::REGULAR:
            return Status::Regular;
        case SPxSolver::Status::RUNNING:
            return Status::InProgress;
        case SPxSolver::Status::UNKNOWN:
            return Status::NotLoaded;
        case SPxSolver::Status::OPTIMAL:
            return Status::Optimal;
        case SPxSolver::Status::UNBOUNDED:
            return Status::Unbounded;
        case SPxSolver::Status::INFEASIBLE:
            return Status::Infeasible;
        case SPxSolver::Status::INForUNBD:
            return Status::InfeasibleOrUnbounded;
        case SPxSolver::Status::OPTIMAL_UNSCALED_VIOLATIONS:
            return Status::OptimalUnscaledViolations;
        default:
            throw UnknownStatusException();
    }
}

}  // namespace lpint