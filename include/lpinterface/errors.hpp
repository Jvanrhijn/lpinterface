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

class UknownStatusException : public LpException {
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

enum class Status {
  //! No solution information available.
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
};

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
