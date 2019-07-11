#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include "lpinterface.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver {

  public:
    // placeholder stuff for now
    GurobiSolver() = default;

    ~GurobiSolver() override = default;

    virtual LpError solve_primal() override;

    virtual LpError solve_dual() override;

};

}

#endif // LPINTERFACE_LPINTERFACE_GUROBI_H
