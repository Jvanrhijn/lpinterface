#ifndef LPINTERFACE_PARAMETER_TYPE_H
#define LPINTERFACE_PARAMETER_TYPE_H

#include "bitmask.hpp"

namespace lpint {

/**
 * @brief Enum class representing linear solver parameters.
 * The linear solvers this library interfaces with have
 * various parameters that are used internally. This enum
 * class represents these parameters in a generic fashion.
 */
enum class Param {
  //! How many threads the solver should use.
  Threads = 1 << 0,
  //! Whether to minimize or maximize the objective.
  ObjectiveSense = 1 << 2,
  //! At what value the objective should be cut off.
  Cutoff = 1 << 3,
  //! Infinity threshold.
  Infinity = 1 << 4,
  //! Time limit alotted to solution process.
  TimeLimit = 1 << 5,
  //! Lower limit of the objective function.
  ObjectiveLowerLimit = 1 << 6,
  //! Upper limit of the objective function.
  ObjectiveUpperLimit = 1 << 7,
  //! Verbosity level. Lower is less verbose, minimum 0.
  Verbosity = 1 << 8,
  //! Whether to solve the primal (0) or dual (1) LP.
  PrimalOrDual = 1 << 9,
  //! Maximum number of iterations for simplex algorithm.
  IterationLimit = 1 << 10,
};

}  // namespace lpint

template <>
struct bitmask::EnableBitmaskOperators<lpint::Param> {
  static constexpr bool enable = true;
};

#endif  // LPINTERFACE_PARAMETER_TYPE_H