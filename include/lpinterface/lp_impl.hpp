#ifndef LPINTERFACE_LP_IMPL_H
#define LPINTERFACE_LP_IMPL_H

#include "common.hpp"
#include "lp.hpp"

#include <vector>

namespace lpint {

class LinearProgram : public LinearProgramInterface {
 public:
  LinearProgram();

  LinearProgram(LinearProgram&&) = default;
  LinearProgram& operator=(LinearProgram&&) = default;

  LinearProgram(const OptimizationType opt_type, const SparseMatrixType sptype);

  template <typename Entry>
  LinearProgram(const OptimizationType opt_type,
                const std::initializer_list<Entry>&& entries)
      : matrix_(std::forward<decltype(entries)>(entries)),
        opt_type_(opt_type),
        initialized_(true) {}

  template <typename Entry>
  LinearProgram(const OptimizationType opt_type, std::vector<Entry>&& entries,
                std::vector<Constraint<double>>&& constraints,
                Objective<double>&& objective)
      : objective_(objective),
        matrix_(std::forward<decltype(entries)>(entries)),
        constraints_(constraints),
        opt_type_(opt_type),
        initialized_(true) {}

  template <typename Entry>
  LinearProgram(const OptimizationType opt_type, std::vector<Entry>&& entries)
      : matrix_(std::forward<decltype(entries)>(entries)),
        opt_type_(opt_type),
        initialized_(true) {}

  ~LinearProgram() = default;

  void add_columns(std::vector<Column<double>>&& columns) override;

  void add_rows(std::vector<Row<double>>&& rows) override;

  void set_matrix(SparseMatrix<double>&& matrix) override;

  const SparseMatrix<double>& matrix() const override;

  SparseMatrix<double>& matrix() override;

  std::size_t num_vars() const override;

  const std::vector<Constraint<double>>& constraints() const override;

  std::vector<Constraint<double>>& constraints() override;

  void add_constraints(
      const std::vector<Constraint<double>>& constraints) override;

  OptimizationType optimization_type() const override;

  void set_objective(const Objective<double>& objective) override;

  const Objective<double>& objective() const override;

  Objective<double>& objective() override;

  bool is_initialized() const override;

  friend std::ostream& operator<<(std::ostream&, const LinearProgram&);

 private:
  Objective<double> objective_;
  SparseMatrix<double> matrix_;
  std::vector<Constraint<double>> constraints_;
  OptimizationType opt_type_;
  bool initialized_;
};

inline std::ostream& operator<<(std::ostream& os, const LinearProgram& lp) {
  if (lp.matrix_.type() != SparseMatrixType::RowWise) {
    throw NotImplementedError();
  }
  os << lp.opt_type_ << " " << lp.objective_ << std::endl;
  const auto n = lp.matrix_.num_entries();
  const auto& entries = lp.matrix_.entries();
  for (std::size_t i = 0; i < n; i++) {
    os << entries[i] << " " << lp.constraints_[i] << std::endl;
  }
  return os;
}

}  // namespace lpint

#endif  // LPINTERFACE_LP_IMPL_H