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

enum ErrorKind {
  LpInterface = 00000,
  Gurobi = 10001,
};

}  // namespace lpint

#endif  // LPINTERFACE_ERRORS_H
