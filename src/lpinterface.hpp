#ifndef LPINTERFACE_LPINTERFACE_H
#define LPINTERFACE_LPINTERFACE_H

#include <vector>

#include "errors.hpp"
#include "data_objects.hpp"

class LinearProgramSolver {

  public:
    virtual ~LinearProgramSolver() = default;

    virtual LpError add_columns(const std::vector<Column>& columns) = 0;

    virtual LpError add_rows() = 0;

    virtual LpError solve() = 0;
};

#endif // LPINTERFACE_LPINTERFACE_H
