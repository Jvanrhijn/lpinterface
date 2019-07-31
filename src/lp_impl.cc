#include "lpinterface/lp_impl.hpp"

namespace lpint {

LinearProgram::LinearProgram() { initialized_ = false; }

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             const SparseMatrixType sptype)
    : matrix_(sptype), opt_type_(opt_type), initialized_(true) {}

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             const std::initializer_list<Row<double>>&& rows)
    : matrix_(std::forward<decltype(rows)>(rows)),
      opt_type_(opt_type),
      initialized_(true) {}

LinearProgram::LinearProgram(
    const OptimizationType opt_type,
    const std::initializer_list<Column<double>>&& columns)
    : matrix_(std::forward<decltype(columns)>(columns)),
      opt_type_(opt_type),
      initialized_(true) {}

void LinearProgram::add_columns(std::vector<Column<double>>&& columns) {
  return matrix_.add_columns(std::move(columns));
}

void LinearProgram::add_rows(std::vector<Row<double>>&& rows) {
  return matrix_.add_rows(std::move(rows));
}

void LinearProgram::set_matrix(const SparseMatrix<double>& matrix) {
  matrix_ = matrix;
}

const SparseMatrix<double>& LinearProgram::matrix() const { return matrix_; }

SparseMatrix<double>& LinearProgram::matrix() { return matrix_; }

const std::vector<Constraint<double>>& LinearProgram::constraints() const {
  return constraints_;
}

void LinearProgram::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  constraints_ = constraints;
}

OptimizationType LinearProgram::optimization_type() const { return opt_type_; }

void LinearProgram::set_objective(const Objective<double>& objective) {
  objective_ = objective;
}

const Objective<double>& LinearProgram::objective() const { return objective_; }

Objective<double>& LinearProgram::objective() { return objective_; }

bool LinearProgram::is_initialized() const { return initialized_; }

}  // namespace lpint
