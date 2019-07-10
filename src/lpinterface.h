#ifndef LPINTERFACE_LPINTERFACE_H
#define LPINTERFACE_LPINTERFACE_H

#include <iostream>

class LinearProgramSolver {

  public:
    virtual ~LinearProgramSolver() = default;

    // placeholder
    virtual int solve() = 0;


};

#endif // LPINTERFACE_LPINTERFACE_H
