#include <gtest/gtest.h>
#include <iostream>
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "mock_lp.hpp"

using namespace lpint;

GurobiSolver create_grb(const LinearProgram& lp) {
  GurobiSolver grb(std::make_shared<LinearProgram>(lp));
  return grb;
}

TEST(Gurobi, SetParameters) {
  auto lp = std::make_shared<MockLinearProgram>();
  EXPECT_CALL(*lp.get(), optimization_type()).Times(1);
  GurobiSolver grb(lp);
  auto err = grb.set_parameter(Param::GrbThreads, 1);
  ASSERT_TRUE(err);
  err = grb.set_parameter(Param::GrbOutputFlag, 0);
  ASSERT_TRUE(err);
}

TEST(Gurobi, FullProblem) {
  LinearProgram lp(
      OptimizationType::Maximize,
      {Row<double>({1, 2, 3}, {0, 1, 2}), Row<double>({1, 1}, {0, 1})});

  std::vector<Constraint<double>> constr = {
      Constraint<double>{Ordering::LEQ, 4.0},
      Constraint<double>{Ordering::GEQ, 1.0}};

  auto err = lp.add_constraints(constr);
  ASSERT_TRUE(err);

  Objective<double> obj{{1.0, 1.0, 2.0},
                        {VarType::Binary, VarType::Binary, VarType::Binary}};
  err = lp.set_objective(obj);
  ASSERT_TRUE(err);

  // Create the Gurobi solver from the given LP
  auto grb = create_grb(lp);
  // Update the internal Gurobi LP
  err = grb.update_program();
  ASSERT_TRUE(err);

  // Solve the primal LP problem
  err = grb.solve_primal();
  ASSERT_TRUE(err);

  // check solution value
  auto err_sol = grb.get_solution();
  ASSERT_TRUE(err_sol);

  auto solution = err_sol.value();
  ASSERT_EQ(solution.values, (std::vector<double>{1.0, 0.0, 1.0}));
  ASSERT_EQ(solution.objective_value, 3.0);
}
