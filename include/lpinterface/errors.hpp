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
class LpException : public std::exception {};

//! Attempt to add rows to a CSC or columns to a CSR matrix.
class MatrixTypeException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Wrong matrix type assumed";
  }
};

//! Attempt to use a feature that is not yet implemented.
class NotImplementedError : public LpException {
 public:
  virtual const char *what() const throw() { return "Feature not implemented"; }
};

//! Attempt to access solution of an unsolved model.
class ModelNotSolvedException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Model has not yet been solved";
  }
};

//! Attempt set a parameter that is not supported by the current backend.
class UnsupportedParameterException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Invalid parameter was supplied";
  }
};

//! Attempt to set a constaint Ordering that is not supported by the current
//! backend.
class UnsupportedConstraintException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Unsupported constraint type was supplied";
  }
};

//! Attempt to add a matrix entry containing duplicate indices.
class InvalidMatrixEntryException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Invalid matrix entry; does your row/column contain duplicate "
           "indices?";
  }
};

//! Attempt to use a variable type that is not supported by the current backend.
class UnsupportedVariableTypeException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Unsupported variable type for this backend";
  }
};

//! Internal error occured in Gurobi.
class GurobiException : public LpException {
 public:
  virtual const char *what() const throw() {
    // TODO find better way to do this
    char *message = new char[100];
    if (!msg_.empty()) {
      snprintf(message, 99, "Error occured in Gurobi, code %d - %s", code_,
               msg_.c_str());
    } else {
      snprintf(message, 99, "Error occured in Gurobi, code %d", code_);
    }
    return message;
  }

 public:
  GurobiException(int code) : code_(code) {}
  GurobiException(int code, const char *msg) : code_(code), msg_(msg) {}

  int code() const { return code_; }

 private:
  int code_;
  std::string msg_;
};

//! Attempt to use a status code that is not supported.
class UnknownStatusException : public LpException {
 public:
  virtual const char *what() const throw() {
    char *message = new char[100];
    snprintf(message, 99, "Unknown status code encountered: %d", code_);
    return message;
  }

 public:
  UnknownStatusException(int code) : code_(code) {}

 private:
  int code_;
};

//! Attempt to use a feature that is not supported by the current backend.
class UnsupportedFeatureException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Feature not available for this solver backend";
  }
};

//! Attempt to use an uninitialized LinearProgram object.
class LinearProgramNotInitializedException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Attempt to access data of un-initialized linear program";
  }
};

//! Failed to set a parameter value.
class FailedToSetParameterException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "Failed to set an LP solver parameter";
  }
};

//! Internal error occured in SoPlex.
class SoplexException : public LpException {
 public:
  virtual const char *what() const throw() {
    return "SoPlex ran into an error solving an LP";
  }
};

/// Enum class representing LP solution status.
enum class Status : int {
  NotLoaded,      /// No Linear Program has been loaded
  NoInformation,  /// Linear program loaded, but no solution information
                  /// available.
  Optimal,        /// Model was solved to optimality, solution available.
  Infeasible,     /// Model was proven to be infeasible.
  InfeasibleOrUnbounded,  /// Model was proven to be either infeasible or
                          /// unbounded.
  Unbounded,              /// Model was proven to be unbounded.
  Cutoff,  /// Optimal objective for model was proven to be worse than the value
           /// specified in the Cutoff parameter.
  IterationLimit,  /// Number of iterations exceeded user-specified iteration
                   /// limit.
  NodeLimit,       /// Total number of branch-and-cut nodes explored exceeded
                   /// user-specified node limit.
  TimeOut,         /// Time limit reached.
  SolutionLimit,   /// Solutions found exceeded solution limit.
  Interrupted,     /// Optimization interrupted by user.
  NumericFailure,  /// Optimizer ran into unrecoverable numerical difficulties.
  SuboptimalSolution,  /// Could not satisfy tolerances; sub-optimal solution is
                       /// available.
  InProgress,          /// Optimization is currently in progress.
  UserObjectiveLimit,  /// User-specified objective limit has been reached.
  NoRatioTester,       /// No ratiotester loaded (SoPlex).
  NoPricer,            /// No pricer loaded (SoPlex).
  NoSolver,            /// No solver loaded.
  ExitDecomposition,   /// Solving process aborted to exit decomposition simplex
                       /// (SoPlex).
  Decomposition,       /// Solving process aborted due to commence decomposition
                       /// simplex (SoPlex).
  Cycling,             /// Solving process aborted due to presence of cycling.
  OptimalUnscaledViolations,  /// Problem solved to optimality, but unscaled
                              /// solution contains violations.
  NotInitialized,             /// Equivalent to SoPlex NOT_INIT status.
  ObjectiveLimit,  /// Solving process aborted as objective limit has been
                   /// reached
  Regular,         /// LP has a usable basis (SoPlex).
};

// LCOV_EXCL_START
inline std::ostream &operator<<(std::ostream &os, const Status &status) {
  os << static_cast<int>(status);
  return os;
}
// LCOV_EXCL_STOP

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
