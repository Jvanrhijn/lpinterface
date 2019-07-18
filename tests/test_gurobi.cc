#include <gtest/gtest.h>
#include <iostream>
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "mock_lp.hpp"

using namespace lpint;

inline GurobiSolver create_grb(const LinearProgram& lp) {
  GurobiSolver grb(std::make_shared<LinearProgram>(lp));
  return grb;
}

TEST(Gurobi, SetParameters) {
  auto lp = std::make_shared<MockLinearProgram>();
  EXPECT_CALL(*lp.get(), optimization_type()).Times(1);
  GurobiSolver grb(lp);
  grb.set_parameter(Param::GrbThreads, 1);
  grb.set_parameter(Param::GrbOutputFlag, 0);
}

TEST(Gurobi, FullProblem) {
  LinearProgram lp(
      OptimizationType::Maximize,
      {Row<double>({1, 2, 3}, {0, 1, 2}), Row<double>({1, 1}, {0, 1})});

  std::vector<Constraint<double>> constr = {
      Constraint<double>{Ordering::LEQ, 4.0},
      Constraint<double>{Ordering::GEQ, 1.0}};

  lp.add_constraints(constr);

  Objective<double> obj{{1.0, 1.0, 2.0},
                        {VarType::Binary, VarType::Binary, VarType::Binary}};
  lp.set_objective(obj);

  // Create the Gurobi solver from the given LP
  auto grb = create_grb(lp);
  // Update the internal Gurobi LP
  grb.update_program();

  // Solve the primal LP problem
  grb.solve_primal();

  // check solution value
  auto solution = grb.get_solution();

  ASSERT_EQ(solution.values, (std::vector<double>{1.0, 0.0, 1.0}));
  ASSERT_EQ(solution.objective_value, 3.0);
}
