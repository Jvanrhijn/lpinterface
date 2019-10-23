#ifndef LPINTERFACE_LPUTIL_CPLEX_H
#define LPINTERFACE_LPUTIL_CPLEX_H

#include <ilcplex/cplex.h>

namespace lpint {

namespace detail {

template <class F, class... Args>
inline void cplex_function_checked(F f, CPXENVptr env, Args&&... args) {
  int status = f(env, std::forward<Args>(args)...);
  if (status) {
    char msg[CPXMESSAGEBUFSIZE];
    CPXgeterrorstring(env, status, msg);
    throw CplexException(status, msg);
  }
}

}

}

#endif // LPINTERFACE_LPUTIL_CPLEX_H