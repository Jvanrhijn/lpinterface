#ifndef LPINTERFACE_LPUTIL_GUROBI_H
#define LPINTERFACE_LPUTIL_GUROBI_H

#include "gurobi_c.h"
#include "lpinterface/errors.hpp"

#include <fcntl.h>
#include <unistd.h>

namespace lpint {

namespace detail {

inline GRBenv* create_gurobi_env() {
  GRBenv* env;
  int saved_stdout = dup(1);
  close(1);
  int new_stdout = open("/dev/null", O_WRONLY);
  
  int err = GRBloadenv(&env, "");

  close(new_stdout);
  new_stdout = dup(saved_stdout);
  close(saved_stdout);

  if (err) {
    throw GurobiException(err, GRBgeterrormsg(env));
  }
  return env;
}

inline GRBmodel* create_gurobi_model(GRBenv* env) {
  GRBmodel* model;
  if (int err = GRBnewmodel(env, &model, nullptr, 0, nullptr, nullptr, nullptr,
                            nullptr, nullptr)) {
    throw GurobiException(err, GRBgeterrormsg(env));
  }
  return model;
}

template <class F, class... Args>
inline void gurobi_function_checked(F f, GRBmodel* g, Args&&... args) {
  if (int error = f(g, std::forward<Args>(args)...)) {
    throw GurobiException(error, GRBgeterrormsg(GRBgetenv(g)));
  }
}

template <class F, class... Args>
inline void gurobi_function_checked(F f, GRBenv* g, Args&&... args) {
  if (int error = f(g, std::forward<Args>(args)...)) {
    throw GurobiException(error, GRBgeterrormsg(g));
  }
}

}  // namespace detail

}  // namespace lpint

#endif  // LPINTERFACE_LPUTIL_GUROBI_H