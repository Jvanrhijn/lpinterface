#ifndef LPINTERFACE_LPINTERFACE_SOPLEX_H
#define LPINTERFACE_LPINTERFACE_SOPLEX_H

#include <memory>

#include "soplex.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"

namespace lpint {

class SoplexSolver : public LinearProgramSolver {
 public:
  SoplexSolver() = default;
  explicit SoplexSolver(OptimizationType optim_type);
  explicit SoplexSolver(std::shared_ptr<LinearProgramInterface> lp);

  ~SoplexSolver() = default;

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  void update_program() override;

  Status solve_primal() override;

  Status solve_dual() override;

  Status solution_status() const override;

  const LinearProgramInterface& linear_program() const override;

  LinearProgramInterface& linear_program() override;

  const Solution<double>& get_solution() const override;

 private:
  soplex::SoPlex soplex_;

  std::shared_ptr<LinearProgramInterface> linear_program_;

  Solution<double> solution_;

  constexpr static soplex::SoPlex::IntParam translate_int_parameter(
      const Param param);
  constexpr static soplex::SoPlex::RealParam translate_real_parameter(
      const Param param);
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_SOPLEX_H