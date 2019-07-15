#ifndef LPINTERFACE_ERRORS_H
#define LPINTERFACE_ERRORS_H

namespace lpint {

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
};

}

#endif  // LPINTERFACE_ERRORS_H
