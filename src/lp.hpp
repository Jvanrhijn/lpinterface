#ifndef LPINTERFACE_LP_H
#define LPINTERFACE_LP_H

#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"

#include "common.hpp"

namespace lpint {

enum class OptimizationType {
  Minimize,
  Maximize,
};

/**
 * @brief Interface representing linear program formulation.
 * This interface represents linear programs of the form
 * \f[
      \{\min, \max\} c^T x, \\
      Ax \leq b,\\
      x \geq 0,
 * \f]
 * where \f$c \in \mathbb{R}^{n}\f$, \f$x \in \mathbb{R}^n\f$,
 * \f$b \in \mathbb{R}^{m}\f$, and \f$A \in \mathbb{R}^{m \times n}\f$.
 * The interface provides methods to modify the LP internally, as well
 * access the LP structure.
 */
class LinearProgramInterface {
 public:
  virtual ~LinearProgramInterface() = default;
  /**
   * @brief Add columns to the LP.
   */
  virtual expected<void, LpError> add_columns(
      const std::vector<Column<double>>& columns) = 0;

  /**
   * @brief Add rows to the LP.
   */
  virtual expected<void, LpError> add_rows(
      const std::vector<Row<double>>& rows) = 0;

  /**
   * @brief Set the linear program matrix.
   */
  virtual expected<void, LpError> set_matrix(
      const SparseMatrix<double> matrix) = 0;

  /**
   * @brief Add a set of constraints to the LP formulation.
   */
  virtual expected<void, LpError> add_constraints(
    const std::vector<Constraint<double>>& constraints) = 0;

  /**
   * @brief Retrieve the optimization type of this LinearProgramInterface.
   * The Optimization type can be either Type::Minimize or
   * Type::Maximize, which correspond to the LP formulations
   * min c^T * x and max c^T * x, respectively.
   */
  virtual OptimizationType optimization_type() const = 0;

  /**
   * @brief Set the objective function to be used.
   */
  virtual expected<void, LpError> set_objective(const Objective& objective) = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H