#include "lpinterface/soplex/lpinterface_soplex.hpp"

namespace lpint {

using namespace soplex;

SoplexSolver::SoplexSolver(OptimizationType optim_type) {
  soplex_.setIntParam(SoPlex::OBJSENSE, optim_type == OptimizationType::Maximize
                                            ? SoPlex::OBJSENSE_MAXIMIZE
                                            : SoPlex::OBJSENSE_MINIMIZE);
}

SoplexSolver::SoplexSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  soplex_.setIntParam(SoPlex::OBJSENSE,
                      lp->optimization_type() == OptimizationType::Maximize
                          ? SoPlex::OBJSENSE_MAXIMIZE
                          : SoPlex::OBJSENSE_MINIMIZE);
}

void SoplexSolver::set_parameter(const Param param, const int value) {
    throw NotImplementedError();
}

void SoplexSolver::set_parameter(const Param param, const double value) {
    throw NotImplementedError();
}

void SoplexSolver::update_program() {
    throw NotImplementedError();
}

Status SoplexSolver::solve_primal() {
    throw NotImplementedError();
}

Status SoplexSolver::solve_dual() {
    throw NotImplementedError();
}

Status SoplexSolver::solution_status() const {
    throw NotImplementedError();
}

const LinearProgramInterface& SoplexSolver::linear_program() const {
    return *linear_program_;
}

 LinearProgramInterface& SoplexSolver::linear_program()  {
    return *linear_program_;
}

const Solution<double>& SoplexSolver::get_solution() const {
    throw NotImplementedError();
}

int SoplexSolver::translate_int_parameter(const Param param) {
    switch (param) {
        case (Param::ObjectiveSense):
          return SoPlex::OBJSENSE;
        default:
          throw UnsupportedParameterException();
    }
}

int SoplexSolver::translate_real_parameter(const Param param) {
    switch (param) {
        case (Param::Infinity):
          return SoPlex::INFTY;
        default:
          throw UnsupportedParameterException();
    }
}

}  // namespace lpint