#ifndef LPINTERFACE_LPINTERFACE_H
#define LPINTERFACE_LPINTERFACE_H

#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"
#include "lp.hpp"

#include "common.hpp"

namespace lpint {

class LinearProgramSolver {
 public:
  virtual ~LinearProgramSolver() = default;

  /**
   * @brief Get immutable access to the underlying Linear Program object
   *
   */
  virtual const LinearProgram& linear_program() const = 0;

  /**
   * @brief Get mutable access to the underlying Linear Program object
   *
   */
  virtual LinearProgram& linear_program() = 0;

  /**
   * @brief Solve the primal linear program
   *
   */
  virtual expected<void, LpError> solve_primal() = 0;

  /**
   * @brief Solve the dual linear program
   *
   */
  virtual expected<void, LpError> solve_dual() = 0;

  /**
   * @brief Get the solution of the linear program
   *
   */
  virtual std::vector<double> get_solution() = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_H
