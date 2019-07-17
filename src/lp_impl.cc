#include "lp_impl.hpp"

namespace lpint {

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             const SparseMatrixType sptype)
    : matrix_(sptype), opt_type_(opt_type) {}

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             std::initializer_list<Row<double>> rows)
    : matrix_(SparseMatrixType::RowWise), opt_type_(opt_type) {
    matrix_.add_rows(rows);
}

LinearProgram::LinearProgram(const OptimizationType opt_type,
                             std::initializer_list<Column<double>> columns)
    : matrix_(SparseMatrixType::ColumnWise), opt_type_(opt_type) {
    matrix_.add_columns(columns);
}

expected<void, LpError> LinearProgram::add_columns(
    const std::vector<Column<double>>& columns) {
  return matrix_.add_columns(columns);
}

expected<void, LpError> LinearProgram::add_rows(
    const std::vector<Row<double>>& rows) {
  return matrix_.add_rows(rows);
}

expected<void, LpError> LinearProgram::set_matrix(
    const SparseMatrix<double> matrix) {
  matrix_ = matrix;
  return expected<void, LpError>();
}

expected<void, LpError> LinearProgram::add_constraints(
    const std::vector<Constraint<double>>& constraints) {
  constraints_ = constraints;
  return expected<void, LpError>();
}

OptimizationType LinearProgram::optimization_type() const { return opt_type_; }

expected<void, LpError> LinearProgram::set_objective(
    const Objective& objective) {
  objective_ = objective;
  return expected<void, LpError>();
}

}  // namespace lpint