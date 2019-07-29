#ifndef LPINTERFACE_LPINTERFACE_GUROBI_H
#define LPINTERFACE_LPINTERFACE_GUROBI_H

#include <memory>

#include "gurobi_c.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lpinterface.hpp"
#include "lpinterface/errors.hpp"

namespace lpint {

class GurobiSolver : public LinearProgramSolver {
 public:
  // delete the default constructor to prevent
  // deleting invalid memory in destructor
  GurobiSolver() = default;
  explicit GurobiSolver(std::shared_ptr<LinearProgramInterface> lp);

  ~GurobiSolver();

  // rule of five: should implement/delete these
  GurobiSolver(const GurobiSolver&) noexcept;
  GurobiSolver(GurobiSolver&&) noexcept;
  GurobiSolver& operator=(GurobiSolver) noexcept;
  GurobiSolver& operator=(GurobiSolver&&) noexcept = delete;

  virtual void set_parameter(const Param param,
                                                const int value) override;

  virtual void set_parameter(const Param param,
                                                const double value) override;

  void update_program() override;

  Status solve_primal() override;

  Status solve_dual() override;

  Status solution_status() const override;

  const LinearProgramInterface& linear_program() const override;

  LinearProgramInterface& linear_program() override;

  const Solution<double>& get_solution() const override;

 private:
  static std::vector<char> convert_variable_type(const std::vector<VarType>& var_types);

  //! The linear program to solve
  std::shared_ptr<LinearProgramInterface> linear_program_;

  //! The gurobi environment object
  GRBenv* gurobi_env_;

  //! The gurobi model object
  GRBmodel* gurobi_model_;

  //! The solution vector
  Solution<double> solution_;

  // copy-and-swap idiom
  friend void swap(GurobiSolver& first, GurobiSolver& second) noexcept {
    using std::swap;
    swap(first.linear_program_, second.linear_program_);
    swap(first.gurobi_env_, second.gurobi_env_);
  }
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_GUROBI_H
