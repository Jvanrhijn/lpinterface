/** @file lp.hpp */
#ifndef LPINTERFACE_LP_H
#define LPINTERFACE_LP_H

#include <iostream>
#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"

#include "common.hpp"

/** \namespace lpint */
namespace lpint {

/// Objective sense for an LP. \ingroup Enumerations
enum class OptimizationType {
  Minimize,  /// Maximize the objective function value.
  Maximize,  /// Minimize the objective function value.
};

// LCOV_EXCL_START
inline std::ostream& operator<<(std::ostream& os, const OptimizationType ot) {
  switch (ot) {
    case (OptimizationType::Maximize):
      os << "Maximize";
      break;
    case (OptimizationType::Minimize):
      os << "Minimize";
      break;
    default:
      // TODO: make a more descriptive exception
      // for this case
      throw UnsupportedFeatureException();
  }
  return os;
}
// LCOV_EXCL_STOP

/**
 * @brief Interface representing linear program formulation.
 * This interface represents linear programs of the form
      {min, max} c^T x,
      Ax <= b,
      x >= 0,
 * where c, x, and b are real-valued vectors, and
 * A is a real-valued matrix.
 * The interface provides methods to modify the LP internally, as well
 * access the LP structure.
 */
class LinearProgramInterface {
 public:
  virtual ~LinearProgramInterface() = default;

  /**
   * @brief Get the number of variables in the LP.
   */
  virtual std::size_t num_vars() const = 0;

  /**
   * @brief Get a const reference to the vector of constraints.
   */
  virtual const std::vector<Constraint<double>>& constraints() const = 0;

  /**
   * @brief Get a reference to the vector of constraints.
   */
  virtual std::vector<Constraint<double>>& constraints() = 0;

  /**
   * @brief Add a set of constraints to the LP formulation.
   */
  virtual void add_constraints(
      std::vector<Constraint<double>>&& constraints) = 0;

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

  /**
   * @brief Get a const reference to the objective function.
   */
  virtual const Objective<double>& objective() const = 0;

  /**
   * @brief Get a reference to the objective function.
   */
  virtual Objective<double>& objective() = 0;

  /**
   * @brief Check whether the LP is initialized.
   *
   * @return true The LP data is ready to be flushed to a solver backend.
   * @return false The LP data cannot be safely flushed to a backend.
   */
  virtual bool is_initialized() const = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H