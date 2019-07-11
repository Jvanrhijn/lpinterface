#ifndef LPINTERFACE_TESTS_MOCK_LP_H
#define LPINTERFACE_TESTS_MOCK_LP_H

#include "data_objects.hpp"
#include "errors.hpp"
#include "lp.hpp"

#include "expected.hpp"
#include "gmock/gmock.h"

#include <vector>

namespace lpint {

class MockLinearProgram : public LinearProgram {
 public:
  MOCK_METHOD1(add_columns,
               tl::expected<void, LpError>(const std::vector<Column>&));
  MOCK_METHOD1(add_rows, tl::expected<void, LpError>(const std::vector<Row>&));
};

}  // namespace lpint

#endif  // LPINTERFACE_TESTS_MOCK_LP_H