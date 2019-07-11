#ifndef LPINTERFACE_LPINTERFACE_H
#define LPINTERFACE_LPINTERFACE_H

#include <vector>

#include "lp.hpp"
#include "errors.hpp"
#include "data_objects.hpp"

namespace lpint {

class LinearProgramSolver {

  public:
    virtual ~LinearProgramSolver() = default;

    /**
     * @brief Get immutable access to the underlying Linear Program object
     * 
     */
    virtual const LinearProgram& linear_program() const = 0;

    /**
     * @brief Get mutable access to the underlying Linear Program object
     * 
     */
    virtual LinearProgram& linear_program() = 0;

    virtual LpError solve_primal() = 0;

    virtual LpError solve_dual() = 0;

    virtual std::vector<double> get_solution() = 0;
};

}

#endif // LPINTERFACE_LPINTERFACE_H
