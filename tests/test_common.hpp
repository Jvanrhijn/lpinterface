#include <rapidcheck.h>

#include "generators.hpp"
#include "testutil.hpp"

#include "lpinterface.hpp"

namespace lpint {

template <class Solver>
void test_timelimit(std::size_t ncols) {
  templated_prop<Solver>("Timeout when timelimit is zero", [=]() {
    auto solver = gen_simple_valid_lp<Solver>(1, ncols);
    solver.set_parameter(lpint::Param::Verbosity, 0);
    solver.set_parameter(lpint::Param::TimeLimit, 0.0);
    const auto status = solver.solve_primal();
    RC_ASSERT(status == lpint::Status::TimeOut);
  });
}

template <class Solver>
void test_full_problem() {
  Solver solver(OptimizationType::Maximize);

  solver.linear_program().set_objective_sense(OptimizationType::Maximize);

  solver.linear_program().set_objective(Objective<double>({1, 1, 2}));

  std::vector<Constraint<double>> constr;
  constr.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 4.0);
  constr.emplace_back(Row<double>({1, 1}, {0, 1}), 1.0, LPINT_INFINITY);

  solver.linear_program().add_constraints(std::move(constr));

  // // Solve the primal LP problem
  auto status = solver.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  //// // check solution value
  auto solution = solver.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
  ASSERT_NEAR(solution.objective_value, 4.0, 1e-15);
}

}
