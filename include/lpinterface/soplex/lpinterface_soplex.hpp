#ifndef LPINTERFACE_LPINTERFACE_SOPLEX_H
#define LPINTERFACE_LPINTERFACE_SOPLEX_H

#include <algorithm>
#include <memory>
#include <unordered_map>

#include "soplex.h"

#include "lpinterface/badge.hpp"
#include "lpinterface/common.hpp"
#include "lpinterface/data_objects.hpp"
#include "lpinterface/errors.hpp"
#include "lpinterface/lp.hpp"
#include "lpinterface/lp_flush_raw_data.hpp"
#include "lpinterface/lpinterface.hpp"
#include "lpinterface/soplex/lphandle_soplex.hpp"

namespace lpint {

class SoplexSolver : public LinearProgramSolver, public FlushRawData<double> {
 public:
  SoplexSolver();

  explicit SoplexSolver(OptimizationType optim_type);

  bool parameter_supported(const Param param) const override;

  void set_parameter(const Param param, const int value) override;

  void set_parameter(const Param param, const double value) override;

  Status solve_primal() override;

  Status solve_dual() override;

  Status solution_status() const override;

  const ILinearProgramHandle& linear_program() const override;

  ILinearProgramHandle& linear_program() override;

  const Solution<double>& get_solution() const override;

  void add_columns(std::vector<double>&& values,
                   std::vector<int>&& start_indices,
                   std::vector<int>&& row_indices, std::vector<double>&& lb,
                   std::vector<double>&& ub) override;
  void add_rows(std::vector<double>&& values, std::vector<int>&& start_indices,
                std::vector<int>&& col_indices, std::vector<double>&& lb,
                std::vector<double>&& ub) override;
  void add_variables(std::vector<double>&& objective_values,
                     std::vector<VarType>&& var_types) override;

  static Status translate_status(const soplex::SPxSolver::Status status);

 private:
  std::shared_ptr<soplex::SoPlex> soplex_;

  LinearProgramHandleSoplex lp_handle_;

  Solution<double> solution_;

  static const std::unordered_map<Param, int> param_dict_;

  static const std::unordered_map<soplex::SPxSolver::Status, Status> status_dict_; 
};

inline Status SoplexSolver::translate_status(const soplex::SPxSolver::Status status) {
  if (status == soplex::SPxSolver::Status::ERROR) {
    throw SoplexException();
  } else {
    return status_dict_.count(status) ? status_dict_.at(status)
                                      : throw UnknownStatusException(status);
  }
}

}  // namespace lpint

#endif  // LPINTERFACE_LPINTERFACE_SOPLEX_H