#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

#include <iostream>

namespace lpint {

enum ErrorKind {
  LpInterface = 00000,
  Gurobi = 10001,
};

/**
 * @brief Enum class representing an error result.
 * The linear program interface can return errors at many
 * points, for various reasons. This enum represents the
 * cause of such an error. Whenever an error occurs, it
 * will be returned from the failing function inside of
 * an expected<T, E>.
 */
enum class LpError : int {
  //! LP solution ran succesfully
  SolveSuccess = ErrorKind::LpInterface,
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
  //! Gurobi errors
  GurobiError = ErrorKind::Gurobi,
};

inline std::ostream& operator<<(std::ostream& s, LpError err) {
  return s << static_cast<int>(err);
}

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
