#ifndef LPINTERFACE_LPINTERFACE_SOPLEX_H
#define LPINTERFACE_LPINTERFACE_SOPLEX_H

#include <algorithm>
#include <memory>

#include "soplex.h"

#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"

namespace lpint {

class SoplexSolver : public LinearProgramSolver, public FlushRawData<double> {
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

  void add_columns(std::vector<double>&& values,
                   std::vector<int>&& start_indices,
                   std::vector<int>&& row_indices, std::vector<Ordering>&& ord,
                   std::vector<double>&& rhs) override;
  void add_rows(std::vector<double>&& values, std::vector<int>&& start_indices,
                std::vector<int>&& col_indices, std::vector<Ordering>&& ord,
                std::vector<double>&& rhs) override;
  void add_variables(std::vector<double>&& objective_values,
                     std::vector<VarType>&& var_types) override;

 private:
  soplex::SoPlex soplex_;

  std::shared_ptr<LinearProgramInterface> linear_program_;

  Solution<double> solution_;

  constexpr static soplex::SoPlex::IntParam translate_int_parameter(
      const Param param);
  constexpr static soplex::SoPlex::RealParam translate_real_parameter(
      const Param param);
  constexpr static Status translate_status(
      const soplex::SPxSolver::Status status);
};

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_SOPLEX_H