#ifndef LPINTERFACE_LPINTERFACE_H
#define LPINTERFACE_LPINTERFACE_H

#include <vector>

#include "common.hpp"
#include "data_objects.hpp"
#include "errors.hpp"
#include "lp.hpp"
#include "parameter_type.hpp"

namespace lpint {

/**
 * @brief Interface to internal linear program solver.
 * This interface is the most important interface within
 * lpinterface. It allows one to generically use various
 * different linear program solvers polymorphically.
 * See the documentation of its methods and the implementations
 * of this interface for usage information.
 */
class LinearProgramSolver {
 public:
  LinearProgramSolver() = default;
  LinearProgramSolver(const LinearProgramSolver&) = default;
  LinearProgramSolver(LinearProgramSolver&&) = default;
  LinearProgramSolver& operator=(const LinearProgramSolver&) = default;
  LinearProgramSolver& operator=(LinearProgramSolver&&) = default;

  virtual ~LinearProgramSolver() = default;

  /**
   * @brief Get immutable access to the underlying Linear Program object.
   */
  virtual const ILinearProgramHandle& linear_program() const = 0;

  /**
   * @brief Get mutable access to the underlying Linear Program object
   */
  virtual ILinearProgramHandle& linear_program() = 0;

  /**
   * @brief Set an integer-valued parameter in the internal LP solver.
   * This method will fail with LpError::UnsupportedParameterError if
   * `param` is not a valid parameter for the internal solver.
   * @param param The parameter to change.
   * @param value The value to which to set the parameter.
   */
  virtual void set_parameter(const Param param, const int value) = 0;

  /**
   * @brief Set a double-valued parameter in the internal LP solver
   * This method will fail with LpError::UnsupportedParameterError if
   * `param` is not a valid parameter for the internal solver.
   * @param param The parameter to change.
   * @param value The value to which to set the parameter.
   */
  virtual void set_parameter(const Param param, const double value) = 0;

  /**
   * @brief Solve the primal linear program.
   */
  virtual Status solve_primal() = 0;

  /**
   * @brief Solve the dual linear program.
   */
  virtual Status solve_dual() = 0;

  /**
   * @brief Query the LP solver for the solution status
   */
  virtual Status solution_status() const = 0;

  /**
   * @brief Get the solution of the linear program.
   * This method will fail with LpError::ModelNotsolvedError
   * if the solution has not (yet) been found.
   */
  virtual const Solution<double>& get_solution() const = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_H
