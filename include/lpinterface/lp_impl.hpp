#ifndef LPINTERFACE_LP_IMPL_H
#define LPINTERFACE_LP_IMPL_H

#include "common.hpp"
#include "lp.hpp"

#include <vector>

namespace lpint {

class LinearProgram : public LinearProgramInterface {
 public:
  LinearProgram();

  LinearProgram(const OptimizationType opt_type, const SparseMatrixType sptype);

  LinearProgram(const OptimizationType opt_type,
                const std::initializer_list<Row<double>>&& rows);

  LinearProgram(const OptimizationType opt_type,
                const std::initializer_list<Column<double>>&& columns);

  ~LinearProgram() = default;

  void add_columns(std::vector<Column<double>>&& columns) override;

  void add_rows(std::vector<Row<double>>&& rows) override;

  void set_matrix(const SparseMatrix<double>& matrix) override;

  const SparseMatrix<double>& matrix() const override;

  const std::vector<Constraint<double>>& constraints() const override;

  void add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  OptimizationType optimization_type() const override;

  void set_objective(const Objective<double>& objective) override;

  const Objective<double>& objective() const override;

  Objective<double>& objective() override;

  bool is_initialized() const override;

 private:
  Objective<double> objective_;
  SparseMatrix<double> matrix_;
  std::vector<Constraint<double>> constraints_;
  OptimizationType opt_type_;
  bool initialized_;

};

}  // namespace lpint

#endif  // LPINTERFACE_LP_IMPL_H