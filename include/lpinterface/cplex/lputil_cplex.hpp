#ifndef LPINTERFACE_LPUTIL_CPLEX_H
#define LPINTERFACE_LPUTIL_CPLEX_H

#include <ilcplex/cplex.h>

namespace lpint {

using CplexEnv = std::remove_pointer<CPXENVptr>::type;
using CplexLp = std::remove_pointer<CPXLPptr>::type;

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

inline CplexLp* create_cplex_problem(std::shared_ptr<CplexEnv> env) {
  int status;
  auto lp = CPXcreateprob(env.get(), &status, "");
  if (status) {
    throw CplexException(status);
  }
  return lp;
}


}

}

#endif // LPINTERFACE_LPUTIL_CPLEX_H