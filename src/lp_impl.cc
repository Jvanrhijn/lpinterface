#include "lp_impl.hpp"

namespace lpint {

LinearProgram::LinearProgram(const OptimizationType opt_type, const SparseMatrixType sptype) : matrix_(sptype), opt_type_(opt_type)  {}

expected<void, LpError> LinearProgram::add_columns(
    const std::vector<Column<double>>& columns) {
  return matrix_.add_columns(columns);
}

expected<void, LpError> LinearProgram::add_rows(
    const std::vector<Row<double>>& rows) {
  return matrix_.add_rows(rows);
}

expected<void, LpError> LinearProgram::set_matrix(const SparseMatrix<double> matrix) {
  matrix_ = matrix;
  return expected<void, LpError>();
}

OptimizationType LinearProgram::optimization_type() const {
    return opt_type_;
}

expected<void, LpError> LinearProgram::set_objective(const Objective& objective) {
    objective_ = objective;
    return expected<void, LpError>();
}

}  // namespace lpint