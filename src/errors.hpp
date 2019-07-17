#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

namespace lpint {

/**
 * @brief Enum class representing an error result.
 * The linear program interface can return errors at many
 * points, for various reasons. This enum represents the
 * cause of such an error. Whenever an error occurs, it
 * will be returned from the failing function inside of
 * an expected<T, E>.
 */
enum class LpError {
  //! LP solution ran succesfully
  SolveSuccess,
  //! Wrong matrix type (column- or row-wise) assumed
  MatrixTypeError,
  //! Feature not (yet) implemented
  NotImplementedError,
  //! Feature not available in this solver backend
  FeatureNotAvailableError,
  //! Model has not yet been solved
  ModelNotSolvedError,
  //! Invalid parameter
  UnsupportedParameterError,
  //! Unsupported constraint type, e.g. Ordering::LEQ in Gurobi
  UnsupportedConstraintError,
  //! Invalid matrix entry, e.g. if duplicates in nonzero indices
  InvalidMatrixEntryError,
  //! Unsupported variable type
  UnsupportedVariableTypeError,
  //! Out of memory
  OutOfMemoryError,
  //! Required parameter not given
  RequiredParameterError,
  //! Invalid argument was provided
  InvalidArgumentError,
  //! Attempted to query an attribute with out-of-range index
  IndexOutOfRangeError,
  //! Internal Gurobi error
  InternalGurobiError,
};

inline LpError convert_gurobi_error(const int gurobi_error) {
  // TODO: add support for all errors
  // https://www.gurobi.com/documentation/8.1/refman/error_codes.html#sec:ErrorCodes
  switch (gurobi_error) {
    case (10001):
      return LpError::OutOfMemoryError;
    case (10002):
      return LpError::RequiredParameterError;
    case (10003):
      return LpError::InvalidArgumentError;
    case (10006):
      return LpError::IndexOutOfRangeError;
    default:
      return LpError::InternalGurobiError;
  }
}

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
