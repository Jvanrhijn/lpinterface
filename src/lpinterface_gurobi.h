#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include "lpinterface.h"

class GurobiSolver : public LinearProgramSolver {

  public:
    // placeholder stuff for now
    GurobiSolver() = default;

    ~GurobiSolver() = default;

    virtual int solve() override;

};


#endif // LPINTERFACE_LPINTERFACE_GUROBI_H
