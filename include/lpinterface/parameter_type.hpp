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
  GrbOutputFlag,
  GrbThreads,
  ObjectiveSense,
  Cutoff,
  Infinity,
  TimeLimit,
  ObjectiveLowerLimit,
  ObjectiveUpperLimit,
  Verbosity,
  PrimalOrDual,
};

}  // namespace lpint

#endif  // LPINTERFACE_PARAMETER_TYPE_H