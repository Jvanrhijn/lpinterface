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
  //! Invalid matrix entry, e.g. if duplicates in nonzero indices
  InvalidMatrixEntryError,
};

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
