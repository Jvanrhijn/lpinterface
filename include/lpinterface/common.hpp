#ifndef LPINTERCACE_COMMON_H
#define LPINTERCACE_COMMON_H

#include <limits>
#include "errors.hpp"

namespace lpint {

//! Constant representing floating point infinity.
constexpr double LPINT_INFINITY = std::numeric_limits<double>::infinity();

}  // namespace lpint

#endif  // LPINTERCACE_COMMON_H