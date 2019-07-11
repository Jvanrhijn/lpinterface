#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include "lpinterface.hpp"

class GurobiSolver : public LinearProgramSolver {

  public:
    // placeholder stuff for now
    GurobiSolver() = default;

    ~GurobiSolver() override = default;

    virtual int solve() override;

};


#endif // LPINTERFACE_LPINTERFACE_GUROBI_H
