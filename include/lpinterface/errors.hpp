#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

#include <iostream>
#include <exception>

namespace lpint {

/**
 * @brief Class for wrapping exception occurring in lpinterface.
 * The linear program interface can return errors at many
 * points, for various reasons. This exception classed is
 * subclassed for each error cause, allowing fine-grained
 * control over various error paths.
 */
class LpException : public std::exception {
  virtual const char *what() const throw() {
    return msg;
  }
  
  const char *msg;

};

class MatrixTypeException : public LpException {
  const char *msg = "Wrong matrix type assumed";
};

class NotImplementedError : public LpException {
  const char *msg = "Feature not implemented";
 };

 class ModelNotSolvedException : public LpException {
   const char *msg = "Model has not yet been solved";
 };

 class UnsupportedParameterException : public LpException {
   const char *msg = "Invalid parameter was supplied";
 };

 class UnsupportedConstraintException : public LpException {
   const char *msg = "Unsupported constraint type was supplied";
 };

 class InvalidMatrixEntryException : public LpException {
   const char *msg = "Invalid matrix entry; does your row/column contain duplicate indices?";
 };

 class UnsupportedVariableTypeException : public LpException {
   const char *msg = "Unsupported variable type for this backend";
 };

 class GurobiException : public LpException {
   virtual const char *what() const throw() {
     return ("Error occured in Gurobi: code " + std::to_string(code_)).c_str();
   }

  public:
    GurobiException(int code) : code_(code) {}

  private:
    int code_;

 };

 class UknownStatusException : public LpException {
   const char *msg = "Unknown status code encountered";
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
   //! Optimization is currently in progress; encountered when optimization is ran
   //! asynchronously.
   InProgress,
   //! User-specified objective limit has been reached.
   UserObjectiveLimit,
 };

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
