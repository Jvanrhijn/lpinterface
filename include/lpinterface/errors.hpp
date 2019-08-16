#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

#include <exception>
#include <iostream>
#include <sstream>

namespace {

// shamelessly stolen from stackoverflow
class Formatter {
 public:
  Formatter() {}
  ~Formatter() {}
  Formatter(Formatter &&) = delete;
  Formatter &operator=(Formatter &&) = delete;

  template <typename Type>
  Formatter &operator<<(const Type &value) {
    stream_ << value;
    return *this;
  }

  std::string str() const { return stream_.str(); }
  operator std::string() const { return stream_.str(); }

  enum ConvertToString { to_str };
  std::string operator>>(ConvertToString) { return stream_.str(); }

 private:
  std::stringstream stream_;

  Formatter(const Formatter &);
  Formatter &operator=(Formatter &);
};

}  // namespace

namespace lpint {

/**
 * @brief Class for wrapping exception occurring in lpinterface.
 * The linear program interface can return errors at many
 * points, for various reasons. This exception classed is
 * subclassed for each error cause, allowing fine-grained
 * control over various error paths.
 */
class LpException : public std::runtime_error {
 public:
  explicit LpException(const char *msg) : std::runtime_error(msg) {}

  template <typename... Args>
  explicit LpException(Args &&... args)
      : std::runtime_error(std::forward<Args>(args)...) {}
};

//! Attempt to add rows to a CSC or columns to a CSR matrix.
class MatrixTypeException : public LpException {
 public:
  MatrixTypeException() : LpException("Wrong matrix type assumed") {}
};

//! Attempt to use a feature that is not yet implemented.
class NotImplementedError : public LpException {
 public:
  NotImplementedError() : LpException("Feature not implemented") {}
};

//! Attempt to access solution of an unsolved model.
class ModelNotSolvedException : public LpException {
 public:
  ModelNotSolvedException() : LpException("Model has not yet been solved") {}
};

//! Attempt set a parameter that is not supported by the current backend.
class UnsupportedParameterException : public LpException {
 public:
  UnsupportedParameterException()
      : LpException("Invalid parameter was supplied") {}
};

//! Attempt to set a constaint Ordering that is not supported by the current
//! backend.
class UnsupportedConstraintException : public LpException {
 public:
  UnsupportedConstraintException()
      : LpException("Unsupported constraint type was supplied") {}
};

//! Attempt to add a matrix entry containing duplicate indices.
class InvalidMatrixEntryException : public LpException {
 public:
  InvalidMatrixEntryException()
      : LpException(
            "Invalid matrix entry; does your row/column contain duplicate "
            "indices?") {}
};

//! Attempt to use a variable type that is not supported by the current backend.
class UnsupportedVariableTypeException : public LpException {
 public:
  UnsupportedVariableTypeException()
      : LpException("Unsupported variable type for this backend") {}
};

//! Internal error occured in Gurobi.
class GurobiException : public LpException {
 public:
  explicit GurobiException(int code)
      : LpException("Error occured in Gurobi, code " + std::to_string(code)),
        code_(code) {}

  GurobiException(int code, const char *msg)
      : LpException("Error occured in Gurobi, code " + std::to_string(code) +
                    "- " + std::string(msg)),
        code_(code) {}

  int code() const { return code_; }

 private:
  int code_;
};

//! Attempt to use a status code that is not supported.
class UnknownStatusException : public LpException {
 public:
  explicit UnknownStatusException(int code)
      : LpException("Unknown status code encountered: " +
                    std::to_string(code)) {}
};

//! Attempt to use a feature that is not supported by the current backend.
class UnsupportedFeatureException : public LpException {
 public:
  UnsupportedFeatureException()
      : LpException("Feature not available for this solver backend") {}
};

//! Attempt to use an uninitialized LinearProgram object.
class LinearProgramNotInitializedException : public LpException {
 public:
  LinearProgramNotInitializedException()
      : LpException("Attempt to access data of un-initialized linear program") {
  }
};

//! Failed to set a parameter value.
class FailedToSetParameterException : public LpException {
 public:
  FailedToSetParameterException()
      : LpException("Failed to set an LP solver parameter") {}
};

//! Internal error occured in SoPlex.
class SoplexException : public LpException {
 public:
  SoplexException() : LpException("SoPlex ran into an error solving an LP") {}
};

class MismatchedDimensionsException : public LpException {
 public:
  MismatchedDimensionsException()
      : LpException("Array dimensions mismatched") {}
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
