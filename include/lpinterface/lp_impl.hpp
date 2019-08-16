#ifndef LPINTERFACE_LP_IMPL_H
#define LPINTERFACE_LP_IMPL_H

#include "common.hpp"
#include "lp.hpp"
#include "lpinterface.hpp"

#include <vector>

namespace lpint {

class LinearProgram : public LinearProgramInterface {
 public:
  LinearProgram(const LinearProgram&) = delete;
  LinearProgram& operator=(const LinearProgram&) = delete;

  LinearProgram(LinearProgram&&) = default;
  LinearProgram& operator=(LinearProgram&&) = default;

  explicit LinearProgram(const OptimizationType opt_type);

  LinearProgram(const OptimizationType opt_type,
                std::vector<Constraint<double>>&& constraints,
                Objective<double>&& objective)
      : objective_(objective),
        constraints_(std::move(constraints)),
        opt_type_(opt_type) {}

  ~LinearProgram() = default;

  std::size_t num_vars() const override;

  const std::vector<Constraint<double>>& constraints() const override;

  std::vector<Constraint<double>>& constraints() override;

  void add_constraints(std::vector<Constraint<double>>&& constraints) override;

  OptimizationType optimization_type() const override;

  void set_objective(const Objective<double>& objective) override;

  const Objective<double>& objective() const override;

  Objective<double>& objective() override;

  bool is_initialized() const override;

  friend std::ostream& operator<<(std::ostream&, const LinearProgram&);

 private:
  Objective<double> objective_;
  std::vector<Constraint<double>> constraints_;
  OptimizationType opt_type_;
};

// LCOV_EXCL_START
inline std::ostream& operator<<(std::ostream& os, const LinearProgram& lp) {
  os << lp.opt_type_ << " " << lp.objective_ << std::endl;
  for (const auto& constraint : lp.constraints_) {
    os << constraint << std::endl;
  }
  return os;
}
// LCOV_EXCL_STOP

}  // namespace lpint

#endif  // LPINTERFACE_LP_IMPL_H