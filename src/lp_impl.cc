#include "lpinterface/lp_impl.hpp"

namespace lpint {

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             const SparseMatrixType sptype)
    : matrix_(sptype), opt_type_(opt_type) {}

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             const std::initializer_list<Row<double>>&& rows)
    : matrix_(std::forward<decltype(rows)>(rows)), opt_type_(opt_type) {}

LinearProgram::LinearProgram(
    const OptimizationType opt_type,
    const std::initializer_list<Column<double>>&& columns)
    : matrix_(std::forward<decltype(columns)>(columns)), opt_type_(opt_type) {}

expected<void, LpError> LinearProgram::add_columns(
    const std::vector<Column<double>>& columns) {
  return matrix_.add_columns(columns);
}

expected<void, LpError> LinearProgram::add_rows(
    const std::vector<Row<double>>& rows) {
  return matrix_.add_rows(rows);
}

expected<void, LpError> LinearProgram::set_matrix(
    const SparseMatrix<double>& matrix) {
  matrix_ = matrix;
  return expected<void, LpError>();
}

const SparseMatrix<double>& LinearProgram::matrix() const { return matrix_; }

const std::vector<Constraint<double>>& LinearProgram::constraints() const {
  return constraints_;
}

expected<void, LpError> LinearProgram::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  constraints_ = constraints;
  return expected<void, LpError>();
}

OptimizationType LinearProgram::optimization_type() const { return opt_type_; }

expected<void, LpError> LinearProgram::set_objective(
    const Objective<double>& objective) {
  objective_ = objective;
  return expected<void, LpError>();
}

const Objective<double>& LinearProgram::objective() const { return objective_; }

}  // namespace lpint
