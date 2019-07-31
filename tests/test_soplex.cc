#include <gtest/gtest.h>
#include <iostream>
#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"
#include "mock_lp.hpp"

using namespace lpint;

inline SoplexSolver create_spl(const LinearProgram& lp) {
  SoplexSolver spl(std::make_shared<LinearProgram>(lp));
  return spl;
}

TEST(Soplex, SetParameters) {
  auto lp = std::make_shared<MockLinearProgram>();
  EXPECT_CALL(*lp.get(), optimization_type()).Times(1);
  SoplexSolver spl(lp);
  spl.set_parameter(Param::GrbThreads, 1);
  spl.set_parameter(Param::GrbOutputFlag, 0);
}

TEST(Soplex, FullProblem) {
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
  auto spl = create_spl(lp);
  // Update the internal Gurobi LP
  spl.update_program();

  // Solve the primal LP problem
  auto status = spl.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  // check solution value
  auto solution = spl.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{1.0, 0.0, 1.0}));
  ASSERT_EQ(solution.objective_value, 3.0);
}

//TEST(Soplex, FullProblemRawData) {
//  // Create the Gurobi solver
//  SoplexSolver spl(OptimizationType::Maximize);
//
//  {
//    std::vector<double> values = {1, 2, 3, 1, 1};
//    std::vector<int> start_indices = {0, 3};
//    std::vector<int> col_indices = {0, 1, 2, 0, 1};
//    std::vector<double> rhs = {4.0, 1.0};
//    std::vector<Ordering> ord = {Ordering::LEQ, Ordering::GEQ};
//    std::vector<double> objective = {1.0, 1.0, 2.0};
//    std::vector<VarType> var_type = {VarType::Real, VarType::Real,
//                                     VarType::Real};
//
//    spl.add_variables(std::move(objective), std::move(var_type));
//    spl.add_rows(std::move(values), std::move(start_indices),
//                 std::move(col_indices), std::move(ord), std::move(rhs));
//  }
//
//  // Solve the primal LP problem
//  auto status = spl.solve_primal();
//  ASSERT_EQ(status, Status::Optimal);
//
//  //// check solution value
//  auto solution = spl.get_solution();
//
//  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
//  ASSERT_EQ(solution.objective_value, 4.0);
//}
//