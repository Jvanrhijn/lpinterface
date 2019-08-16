#ifndef LPINTERCACE_COMMON_H
#define LPINTERCACE_COMMON_H

#include <limits>
#include "errors.hpp"

namespace lpint {

//! Constant representing floating point infinity.
constexpr double LPINT_INFINITY = std::numeric_limits<double>::infinity();

namespace detail {

constexpr long CPP_14 = 201402L;

}

}  // namespace lpint

#endif  // LPINTERCACE_COMMON_H