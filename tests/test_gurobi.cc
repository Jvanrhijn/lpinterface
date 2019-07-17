#include <gtest/gtest.h>
#include <iostream>
#include "lp_impl.hpp"
#include "lpinterface_gurobi.hpp"

using namespace lpint;

GurobiSolver create_grb(const LinearProgram& lp) {
  GurobiSolver grb(std::make_shared<LinearProgram>(lp));
  return grb;
}

TEST(Gurobi, SetParameters) {
  LinearProgram lp(OptimizationType::Maximize,
                   {Row<double>({1.0, 1.0, 2.0}, {0, 1, 2}),
                    Row<double>({1.0, 2.0, 3.0}, {0, 1, 2}),
                    Row<double>({1, 0, 1.0}, {0, 1})});
  auto grb = create_grb(lp);
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
  auto grb = create_grb(lp);
  err = grb.update_program();
  ASSERT_TRUE(err);
}