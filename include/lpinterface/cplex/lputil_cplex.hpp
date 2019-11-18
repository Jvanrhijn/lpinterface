#ifndef LPINTERFACE_LPUTIL_CPLEX_H
#define LPINTERFACE_LPUTIL_CPLEX_H

#include <ilcplex/cplex.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "lpinterface/errors.hpp"

namespace lpint {

using CplexEnv = std::remove_pointer<CPXENVptr>::type;
using CplexLp = std::remove_pointer<CPXLPptr>::type;

namespace detail {

template <class F, class... Args>
inline void cplex_function_checked(F f, CPXENVptr env, Args&&... args) {
  int status = f(env, std::forward<Args>(args)...);
  if (status) {
    std::array<char, CPXMESSAGEBUFSIZE> msg = {0};
    CPXgeterrorstring(env, status, msg.data());
    throw CplexException(status, msg.data());
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

}  // namespace detail

}  // namespace lpint

#endif  // LPINTERFACE_LPUTIL_CPLEX_H