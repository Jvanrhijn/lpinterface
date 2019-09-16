#include "lpinterface/soplex/lphandle_soplex.hpp"
#include "lpinterface/detail/util.hpp"

namespace lpint {

using namespace soplex;

std::vector<Variable> LinearProgramHandleSoplex::variables() const {
  std::vector<Variable> vars;
  for (const auto i : inverse_permutation_vars_) {
    vars.emplace_back(soplex_->lowerReal(i), soplex_->upperReal(i));
  }
  return vars;
}

void LinearProgramHandleSoplex::add_variables(
    const std::vector<Variable>& vars) {
  DSVector dummy(0);
  for (const auto& var : vars) {
    soplex_->addColReal(LPCol(0, dummy, var.upper(), var.lower()));
    permutation_vars_.push_back(permutation_vars_.size());
    inverse_permutation_vars_.push_back(inverse_permutation_vars_.size());
  }
}

void LinearProgramHandleSoplex::add_variables(const std::size_t nvars) {
  add_variables(std::vector<Variable>(nvars));
}

void LinearProgramHandleSoplex::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  for (auto& constraint : constraints) {
    DSVector ds_row(constraint.row.num_nonzero());
    ds_row.add(constraint.row.num_nonzero(),
               constraint.row.nonzero_indices().data(),
               constraint.row.values().data());
    soplex_->addRowReal(
        LPRow(constraint.lower_bound, ds_row, constraint.upper_bound));
    // add new indices to permutation and inverse permutation.
    // newly added constraints are not permuted, so just add
    // their actual index.
    permutation_.push_back(permutation_.size());
    inverse_permutation_.push_back(inverse_permutation_.size());
  }
}

void LinearProgramHandleSoplex::remove_variable(const std::size_t i) {
  soplex_->removeColReal(inverse_permutation_vars_[i]);
  std::swap(permutation_vars_[inverse_permutation_vars_[i]],
            permutation_vars_.back());
  permutation_vars_.pop_back();
  permutation_vars_ = detail::rankify(permutation_vars_);
  inverse_permutation_vars_ = detail::inverse_permutation(permutation_vars_);
}

void LinearProgramHandleSoplex::remove_constraint(const std::size_t i) {
  soplex_->removeRowReal(inverse_permutation_[i]);
  // calculate the new permutation and inverse permutation. Soplex removed
  // constraints by swapping them with then end of the constraint list
  // and shrinking the list.
  // first, swap the permutation index with the back of its array
  std::swap(permutation_[inverse_permutation_[i]], permutation_.back());
  // shrink the permutation list
  permutation_.pop_back();
  // make sure all permutation indices are in the range [0, num_constraints]
  permutation_ = detail::rankify(permutation_);
  // compute the inverse of the updated permutation
  inverse_permutation_ = detail::inverse_permutation(permutation_);
}

void LinearProgramHandleSoplex::set_objective(
    const Objective<double>& objective) {
  if (num_vars() != objective.values.size()) {
    throw MismatchedDimensionsException();
  }
  VectorReal obj(static_cast<int>(objective.values.size()),
                 const_cast<Objective<double>&>(objective).values.data());
  soplex_->changeObjReal(obj);
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
  std::vector<Constraint<double>> constraints;
  // to retrieve constraints in the proper order,
  // iterate over the inverse permutation indices.
  for (auto i : inverse_permutation_) {
    auto lb = soplex_->lhsReal(i);
    auto ub = soplex_->rhsReal(i);

    Row<double> row;
    const auto sv = soplex_->rowVectorRealInternal(i);
    for (std::size_t j = 0; j < static_cast<std::size_t>(sv.size()); j++) {
      const auto element = sv.element(j);
      row.nonzero_indices().push_back(element.idx);
      row.values().push_back(element.val);
    }
    constraints.emplace_back(std::move(row), lb, ub);
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