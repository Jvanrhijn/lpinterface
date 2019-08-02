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
  virtual void add_columns(std::vector<Column<double>>&& columns) = 0;

  /**
   * @brief Add rows to the LP.
   */
  virtual void add_rows(std::vector<Row<double>>&& rows) = 0;

  /**
   * @brief Set the linear program matrix.
   */
  virtual void set_matrix(const SparseMatrix<double>& matrix) = 0;

  /**
   * @brief Get immutable access to the constraint matrix
   */
  virtual const SparseMatrix<double>& matrix() const = 0;

  /**
   * @brief Get mutable access to the constraint matrix
   */
  virtual SparseMatrix<double>& matrix() = 0;

  virtual const std::vector<Constraint<double>>& constraints() const = 0;

  virtual std::vector<Constraint<double>>& constraints() = 0;

  /**
   * @brief Add a set of constraints to the LP formulation.
   */
  virtual void add_constraints(
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
  virtual void set_objective(const Objective<double>& objective) = 0;

  virtual const Objective<double>& objective() const = 0;

  virtual Objective<double>& objective() = 0;

  virtual bool is_initialized() const = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H