#ifndef LPINTERFACE_LPUTIL_GUROBI_H
#define LPINTERFACE_LPUTIL_GUROBI_H

#include "gurobi_c.h"

namespace lpint {

namespace detail {

inline GRBenv* create_gurobi_env() {
  GRBenv* env;
  GRBloadenv(&env, "");
  return env;
}

inline GRBmodel* create_gurobi_model(GRBenv* env) {
  GRBmodel* model;
  GRBnewmodel(env, &model, nullptr, 0, nullptr, nullptr, nullptr, nullptr,
              nullptr);
  return model;
}

template <class F, class... Args>
void gurobi_function_checked(F f, GRBmodel* g, Args... args) {
  if (int error = f(g, std::forward<Args>(args)...)) {
    throw GurobiException(error, GRBgeterrormsg(GRBgetenv(g)));
  }
}

template <class F, class... Args>
void gurobi_function_checked(F f, GRBenv* g, Args... args) {
  if (int error = f(g, std::forward<Args>(args)...)) {
    throw GurobiException(error, GRBgeterrormsg(g));
  }
}

}

}

#endif // LPINTERFACE_LPUTIL_GUROBI_H