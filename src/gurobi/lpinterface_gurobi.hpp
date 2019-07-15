#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include <memory>

#include "gurobi_c.h"

#include "lp.hpp"
#include "lpinterface.hpp"

#include "data_objects.hpp"
#include "common.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver {
 public:
  GurobiSolver(LinearProgram&& lp);

  ~GurobiSolver();

  // rule of five: should implement/delete these
  GurobiSolver(const GurobiSolver&) = delete;
  GurobiSolver(GurobiSolver&&) = delete;
  GurobiSolver operator=(const GurobiSolver&) = delete;
  GurobiSolver operator=(GurobiSolver&&) = delete;

  virtual expected<void, LpError> set_parameter(const Param param, const int value);

  virtual expected<void, LpError> set_parameter(const Param param, const double value);

  expected<void, LpError> update_program() override;

  expected<void, LpError> solve_primal() override;

  expected<void, LpError> solve_dual() override;

  const LinearProgram& linear_program() const override;

  LinearProgram& linear_program() override;

  expected<std::vector<double>, LpError> get_solution() const override;

 private:
  //! The linear program to solve
  std::shared_ptr<LinearProgram> linear_program_;

  //! The gurobi environment object
  std::shared_ptr<GRBenv> gurobi_env_;

  //! The gurobi model object
  std::shared_ptr<GRBmodel> gurobi_model_;
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_GUROBI_H
