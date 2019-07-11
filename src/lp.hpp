#ifndef LPINTERFACE_LP_H
#define LPINTERFACE_LP_H

#include <vector>

#include "errors.hpp"
#include "data_objects.hpp"

namespace lpint {

class LinearProgram {

  public:
    virtual ~LinearProgram() = default;

    virtual LpError add_columns(const std::vector<Column>& columns) = 0;

    virtual LpError add_rows(const std::vector<Row>& rows) = 0;
};

}

#endif // LPINTERFACE_LP_H