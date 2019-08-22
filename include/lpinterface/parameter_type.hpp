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
  Threads = 0x0001,
  //! Whether to minimize or maximize the objective.
  ObjectiveSense = 0x0002,
  //! At what value the objective should be cut off.
  Cutoff = 0x0004,
  //! Infinity threshold.
  Infinity = 0x0008,
  //! Time limit alotted to solution process.
  TimeLimit = 0x010,
  //! Lower limit of the objective function.
  ObjectiveLowerLimit = 0x020,
  //! Upper limit of the objective function.
  ObjectiveUpperLimit = 0x040,
  //! Verbosity level. Lower is less verbose, minimum 0.
  Verbosity = 0x080,
  //! Whether to solve the primal (0) or dual (1) LP.
  PrimalOrDual = 0x100,
  //! Maximum number of iterations for simplex algorithm.
  IterationLimit = 0x200,
};

}  // namespace lpint

template <>
struct bitmask::EnableBitmaskOperators<lpint::Param> {
  static constexpr bool enable = true;
};

#endif  // LPINTERFACE_PARAMETER_TYPE_H