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
  //! How many threads the solver should use. (Gurobi)
  Threads,
  //! Whether to minimize or maximize the objective. (Gurobi, SoPlex)
  ObjectiveSense,
  //! At what value the objective should be cut off. (SoPlex)
  Cutoff,
  //! Infinity threshold. (SoPlex)
  Infinity,
  //! Time limit alotted to solution process. (Gurobi, SoPlex)
  TimeLimit,
  //! Lower limit of the objective function. (SoPlex)
  ObjectiveLowerLimit,
  //! Upper limit of the objective function. (SoPlex)
  ObjectiveUpperLimit,
  //! Verbosity level. Lower is less verbose, minimum 0. (Gurobi, SoPlex)
  Verbosity,
  //! Whether to solve the primal (0) or dual (1) LP. (Gurobi, SoPlex)
  PrimalOrDual,
  //! Maximum number of iterations for simplex algorithm. (Gurobi, SoPlex)
  IterationLimit,
};

}  // namespace lpint

#endif  // LPINTERFACE_PARAMETER_TYPE_H