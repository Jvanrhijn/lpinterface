#ifndef LPINTERFACE_PARAMETER_TYPE_H
#define LPINTERFACE_PARAMETER_TYPE_H

namespace lpint {

/**
 * @brief Enum class representing linear solver parameters.
 * The linear solvers this library interfaces with have
 * various parameters that are used internally. This enum
 * class represents these parameters in a generic fashion.
 */
enum class Param {
  //! How many threads the solver should use.
  Threads,
  //! Whether to minimize or maximize the objective.
  ObjectiveSense,
  //! At what value the objective should be cut off.
  Cutoff,
  //! Infinity threshold.
  Infinity,
  //! Time limit alotted to solution process.
  TimeLimit,
  //! Lower limit of the objective function.
  ObjectiveLowerLimit,
  //! Upper limit of the objective function.
  ObjectiveUpperLimit,
  //! Verbosity level. Lower is less verbose, minimum 0.
  Verbosity,
  //! Whether to solve the primal (0) or dual (1) LP.
  PrimalOrDual,
};

}  // namespace lpint

#endif  // LPINTERFACE_PARAMETER_TYPE_H