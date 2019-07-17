#ifndef LPINTERFACE_LP_IMPL_H
#define LPINTERFACE_LP_IMPL_H

#include "common.hpp"
#include "lp.hpp"

#include <vector>

namespace lpint {

class LinearProgram : public LinearProgramInterface {
 public:
  LinearProgram(const OptimizationType opt_type, const SparseMatrixType sptype);

  LinearProgram(const OptimizationType opt_type,
                const std::initializer_list<Row<double>>&& rows);

  LinearProgram(const OptimizationType opt_type,
                const std::initializer_list<Column<double>>&& columns);

  ~LinearProgram() = default;

  expected<void, LpError> add_columns(
      const std::vector<Column<double>>& columns) override;

  expected<void, LpError> add_rows(
      const std::vector<Row<double>>& rows) override;

  expected<void, LpError> set_matrix(
      const SparseMatrix<double> matrix) override;

  const SparseMatrix<double>& matrix() const override;

  const std::vector<Constraint<double>>& constraints() const override;

  expected<void, LpError> add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  OptimizationType optimization_type() const override;

  expected<void, LpError> set_objective(
      const Objective<double>& objective) override;

  const Objective<double>& objective() const override;

 private:
  Objective<double> objective_;
  SparseMatrix<double> matrix_;
  std::vector<Constraint<double>> constraints_;
  OptimizationType opt_type_;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_IMPL_H