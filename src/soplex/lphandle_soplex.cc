#include "lpinterface/soplex/lphandle_soplex.hpp"

namespace lpint {

using namespace soplex;

void LinearProgramHandleSoplex::add_constraints(std::vector<Constraint<double>>&& constraints) {
  for (auto& constraint : constraints) {
      DSVector ds_row(constraint.row.num_nonzero());
      ds_row.add(constraint.row.num_nonzero(),
                 constraint.row.nonzero_indices().data(),
                 constraint.row.values().data());
      soplex_->addRowReal(LPRow(constraint.lower_bound, ds_row, constraint.upper_bound));
  }
}

void LinearProgramHandleSoplex::set_objective(const Objective<double>& objective) {
    DSVector dummy(0);
    for (const auto& coefficient : objective.values) {
        soplex_->addColReal(LPCol(coefficient, dummy, infinity, 0.0));
    }
}

void LinearProgramHandleSoplex::set_objective_sense(const OptimizationType objsense) {
    sense_ = objsense;
    if(!soplex_->setIntParam(SoPlex::OBJSENSE, objsense == OptimizationType::Maximize
                                                     ? SoPlex::OBJSENSE_MAXIMIZE : SoPlex::OBJSENSE_MINIMIZE)) {
                                                         throw FailedToSetParameterException();
                                                     }
}

OptimizationType LinearProgramHandleSoplex::optimization_type() const {
    return sense_;
}

std::size_t LinearProgramHandleSoplex::num_vars() const {
    return static_cast<std::size_t>(soplex_->numColsReal());

}

} // namespace lpint