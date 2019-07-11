#ifndef LPINTERFACE_TESTS_MOCK_LP_H
#define LPINTERFACE_TESTS_MOCK_LP_H

#include "lp.hpp"
#include "data_objects.hpp"
#include "errors.hpp"
#include "gmock/gmock.h"

#include <vector>

namespace lpint {

class MockLinearProgram : public LinearProgram {

  public:
    MOCK_METHOD1(add_columns, LpError(const std::vector<Column>&));
    MOCK_METHOD1(add_rows, LpError(const std::vector<Row>&));

};

}

#endif // LPINTERFACE_TESTS_MOCK_LP_H