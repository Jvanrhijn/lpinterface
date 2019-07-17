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
 * this interface for usage information.
 */
class LinearProgramSolver {
 public:
  LinearProgramSolver() = default;

  virtual ~LinearProgramSolver() = default;

  /**
   * @brief Get immutable access to the underlying Linear Program object.
   */
  virtual const LinearProgramInterface& linear_program() const = 0;

  /**
   * @brief Get mutable access to the underlying Linear Program object
   */
  virtual LinearProgramInterface& linear_program() = 0;

  /**
   * @brief Set an integer-valued parameter in the internal LP solver.
   * This method will fail with LpError::UnsupportedParameterError if
   * `param` is not a valid parameter for the internal solver.
   * @param param The parameter to change.
   * @param value The value to which to set the parameter.
   */
  virtual expected<void, LpError> set_parameter(const Param param,
                                                const int value) = 0;

  /**
   * @brief Set a double-valued parameter in the internal LP solver
   * This method will fail with LpError::UnsupportedParameterError if
   * `param` is not a valid parameter for the internal solver.
   * @param param The parameter to change.
   * @param value The value to which to set the parameter.
   */
  virtual expected<void, LpError> set_parameter(const Param param,
                                                const double value) = 0;

  /**
   * @brief Update the linear program in the internal LP solver.
   * Use this method after changing configuration of the LinearProgram
   * object contained in this object.
   */
  virtual expected<void, LpError> update_program() = 0;

  /**
   * @brief Solve the primal linear program.
   */
  virtual expected<void, LpError> solve_primal() = 0;

  /**
   * @brief Solve the dual linear program.
   */
  virtual expected<void, LpError> solve_dual() = 0;

  /**
   * @brief Get the solution of the linear program.
   * This method will fail with LpError::ModelNotsolvedError
   * if the solution has not (yet) been found.
   */
  virtual expected<Solution<double>, LpError> get_solution() const = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_H
