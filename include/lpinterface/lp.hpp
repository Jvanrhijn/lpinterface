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
  //! Maximize the objective function value.
  Minimize,
  //! Minimize the objective function value.
  Maximize,
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
class ILinearProgramHandle {
 public:
  ILinearProgramHandle() = default;
  ILinearProgramHandle(const ILinearProgramHandle&) = default;
  ILinearProgramHandle(ILinearProgramHandle&&) = default;
  ILinearProgramHandle& operator=(const ILinearProgramHandle&) = default;
  ILinearProgramHandle& operator=(ILinearProgramHandle&&) = default;

  virtual ~ILinearProgramHandle() = default;

  /**
   * @brief Get the number of variables in the LP.
   */
  virtual std::size_t num_vars() const = 0;

  /**
   * @brief Get the number of cosntraints in the LP.
   */
  virtual std::size_t num_constraints() const = 0;

  /**
   * @brief Set the objective sense of this ILinearProgramHandle.
   * The Optimization type can be either OptimizationType::Minimize or
   * OptimizationType::Maximize, which correspond to the LP formulations
   * min c^T * x and max c^T * x, respectively.
   */
  virtual void set_objective_sense(const OptimizationType objsense) = 0;

  /**
   * @brief Retrieve variable i from the internal LP solver.
   *
   * @param i Index of variable to retrieve.
   * @return std::vector<Variable> Vector containing internal LP variables.
   */
  virtual Variable variable(std::size_t i) const = 0;

  /**
   * @brief Retrieve the variables from the internal LP solver.
   *
   * @return std::vector<Variable> Vector containing internal LP variables.
   */
  virtual std::vector<Variable> variables() const = 0;

  /**
   * @brief Add variables to the LP.
   *
   * @param vars Vector of variables to add.
   */
  virtual void add_variables(const std::vector<Variable>& vars) = 0;

  /**
   * @brief Add num_vars non-negative variables to the LP.
   * To be called before settings the objective function.
   */
  virtual void add_variables(const std::size_t num_vars) = 0;

  /**
   * @brief Add a set of constraints to the LP formulation. This
   * can only be called after calling set_objective().
   */
  virtual void add_constraints(
      const std::vector<Constraint<double>>& constraints) = 0;

  /**
   * @brief Remove a constraint from the LP.
   *
   * @param i Index of the constraint to remove.
   */
  virtual void remove_constraint(const std::size_t i) = 0;

  /**
   * @brief Remove a variable from the LP.
   *
   * @param i Index of variable to remove.
   */
  virtual void remove_variable(const std::size_t i) = 0;

  /**
   * @brief Retrieve the objective sense of this ILinearProgramHandle.
   * The Optimization type can be either OptimizationType::Minimize or
   * OptimizationType::Maximize, which correspond to the LP formulations
   * min c^T * x and max c^T * x, respectively.
   */
  virtual OptimizationType optimization_type() const = 0;

  /**
   * @brief Set the objective function to be used. This method
   * must be called before calling add_constraints().
   */
  virtual void set_objective(const Objective<double>& objective) = 0;

  /**
   * @brief Retrieve constraint i of the internal LP.
   * This method requests a constraint from the internal LP
   * solver backend, copies it, and returns it.
   * Since constraints have to be copied from the backend,
   * this can be an expensive operation, and so should be
   * used sparingly.
   *
   * @param i Index of constraint.
   * @return Constraint<double>
   */
  virtual Constraint<double> constraint(std::size_t i) const = 0;

  /**
   * @brief Retrieve the constraints of the internal LP.
   * This method requests the constraints from the internal LP
   * solver backend, copies them, and returns them in a vector.
   * Since the constraints have to be copied from the backend,
   * this can be quite an expensive operation, and so should
   * not be called in a loop.
   *
   * @return std::vector<Constraint<double>>
   */
  virtual std::vector<Constraint<double>> constraints() const = 0;

  /**
   * @brief Retrieve the objective function of the internal LP.
   * This method requests the objective function values from
   * the LP backend, copies them, and returns them. Since the
   * objective values have to be copied from the backend, this
   * can be quite an expensive operation, and so should not be
   * called in a loop.
   *
   * @return Objective<double>
   */
  virtual Objective<double> objective() const = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H