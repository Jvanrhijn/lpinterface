#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

#include <exception>
#include <iostream>

namespace lpint {

/**
 * @brief Class for wrapping exception occurring in lpinterface.
 * The linear program interface can return errors at many
 * points, for various reasons. This exception classed is
 * subclassed for each error cause, allowing fine-grained
 * control over various error paths.
 */
class LpException : public std::exception {
 public:
  virtual const char *what() const throw() { return msg; }

  const char *msg;
};

class MatrixTypeException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Wrong matrix type assumed";
  }
};

class NotImplementedError : public LpException {
 public:
  virtual const char *what() const throw() { return "Feature not implemented"; }
};

class ModelNotSolvedException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Model has not yet been solved";
  }
};

class UnsupportedParameterException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Invalid parameter was supplied";
  }
};

class UnsupportedConstraintException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Unsupported constraint type was supplied";
  }
};

class InvalidMatrixEntryException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Invalid matrix entry; does your row/column contain duplicate "
           "indices?";
  }
};

class UnsupportedVariableTypeException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Unsupported variable type for this backend";
  }
};

class GurobiException : public LpException {
 public:
  virtual const char *what() const throw() {
    // TODO find better way to do this
    char *message = new char[100];
    snprintf(message, 99, "Error occured in Gurobi, code %d", code_);
    return message;
  }

 public:
  GurobiException(int code) : code_(code) {}

 private:
  int code_;
};

class UnknownStatusException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Unknown status code encountered";
  }
};

class UnsupportedFeatureException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Feature not available for this solver backend";
  }
};

class LinearProgramNotInitializedException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Attempt to access data of un-initialized linear program";
  }
};

class FailedToSetParameterException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Failed to set an LP solver parameter";
  }
};

class SoplexException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "SoPlex ran into an error solving an LP";
  }
};

enum class Status {
  //! No Linear Program has been loaded
  NotLoaded,
  //! Linear program loaded, but no solution information available.
  NoInformation,
  //! Model was solved to optimality, solution available.
  Optimal,
  //! Model was proven to be infeasible.
  Infeasible,
  //! Model was proven to be either infeasible or unbounded.
  InfeasibleOrUnbounded,
  //! Model was proven to be unbounded.
  Unbounded,
  //! Optimal objective for model was proven to be worse
  //! than the value specified in the Cutoff parameter.
  Cutoff,
  //! Number of iterations exceeded user-specified iteration limit.
  IterationLimit,
  //! Total number of branch-and-cut nodes explored exceeded user-specified
  //! node limit.
  NodeLimit,
  //! Time limit reached.
  TimeOut,
  //! Solutions found exceeded solution limit.
  SolutionLimit,
  //! Optimization interrupted by user.
  Interrupted,
  //! Optimizer ran into unrecoverable numerical difficulties.
  NumericFailure,
  //! Could not satisfy tolerances; sub-optimal solution is available.
  SuboptimalSolution,
  //! Optimization is currently in progress; encountered when optimization is
  //! ran
  //! asynchronously.
  InProgress,
  //! User-specified objective limit has been reached.
  UserObjectiveLimit,
  //! No ratiotester loaded (SoPlex).
  NoRatioTester,
  //! No pricer loaded (SoPlex).
  NoPricer,
  //! No solver loaded.
  NoSolver,
  //! Solving process aborted to exit decomposition simplex (SoPlex).
  ExitDecomposition,
  //! Solving process aborted due to commence decomposition simplex (SoPlex).
  Decomposition,
  //! Solving process aborted due to presence of cycling.
  Cycling,
  //! Problem solved to optimality, but unscaled solution contains violations.
  OptimalUnscaledViolations,
  //! Equivalent to SoPlex NOT_INIT status; unclear from docs what
  //! this actually means. It might be better to just throw an exception here?
  NotInitialized,
  //! Solving process aborted as objective limit has been reached
  ObjectiveLimit,
  //! LP has a usable basis (SoPlex), unclear from soplex docs what this
  //! means.
  Regular,
};

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
