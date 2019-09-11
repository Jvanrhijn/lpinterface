#include "lpinterface/soplex/lphandle_soplex.hpp"
#include "util/container.hpp"

namespace lpint {

using namespace soplex;

void LinearProgramHandleSoplex::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  for (auto& constraint : constraints) {
    DSVector ds_row(constraint.row.num_nonzero());
    ds_row.add(constraint.row.num_nonzero(),
               constraint.row.nonzero_indices().data(),
               constraint.row.values().data());
    soplex_->addRowReal(
        LPRow(constraint.lower_bound, ds_row, constraint.upper_bound));
  }
}

void LinearProgramHandleSoplex::remove_constraint(const std::size_t i) {
  soplex_->removeRowReal(i);
}

void LinearProgramHandleSoplex::set_objective(
    const Objective<double>& objective) {
  DSVector dummy(0);
  for (const auto& coefficient : objective.values) {
    soplex_->addColReal(LPCol(coefficient, dummy, infinity, 0.0));
  }
}

void LinearProgramHandleSoplex::set_objective_sense(
    const OptimizationType objsense) {
  sense_ = objsense;
  if (!soplex_->setIntParam(SoPlex::OBJSENSE,
                            objsense == OptimizationType::Maximize
                                ? SoPlex::OBJSENSE_MAXIMIZE
                                : SoPlex::OBJSENSE_MINIMIZE)) {
    throw FailedToSetParameterException();
  }
}

OptimizationType LinearProgramHandleSoplex::optimization_type() const {
  return sense_;
}

std::size_t LinearProgramHandleSoplex::num_vars() const {
  return static_cast<std::size_t>(soplex_->numColsReal());
}

std::size_t LinearProgramHandleSoplex::num_constraints() const {
  return static_cast<std::size_t>(soplex_->numRowsReal());
}

std::vector<Constraint<double>> LinearProgramHandleSoplex::constraints() const {
  const auto nrows = static_cast<std::size_t>(soplex_->numRowsReal());
  std::vector<Constraint<double>> constraints(num_constraints());
  for (std::size_t i = 0; i < nrows; i++) {
    auto idx = i;
    auto lb = soplex_->lhsReal(idx);
    auto ub = soplex_->rhsReal(idx);

    Row<double> row;
    const auto sv = soplex_->rowVectorRealInternal(idx);
    for (std::size_t j = 0; j < static_cast<std::size_t>(sv.size()); j++) {
      const auto element = sv.element(j);
      row.nonzero_indices().push_back(element.idx);
      row.values().push_back(element.val);
    }
    constraints[i] = Constraint<double>(std::move(row), lb, ub);
  }
  return constraints;
}

Objective<double> LinearProgramHandleSoplex::objective() const {
  const auto nvars = num_vars();
  std::vector<double> values;
  for (std::size_t i = 0; i < nvars; i++) {
    values.push_back(soplex_->objReal(i));
  }
  return Objective<double>(std::move(values));
}

}  // namespace lpint