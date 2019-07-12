#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include "lpinterface.hpp"

#include "common.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver {
 public:
  // placeholder stuff for now
  GurobiSolver() = default;

  ~GurobiSolver() override = default;

  virtual expected<void, LpError> solve_primal() override;

  virtual expected<void, LpError> solve_dual() override;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_GUROBI_H
