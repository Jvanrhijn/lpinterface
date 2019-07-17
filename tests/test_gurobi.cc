#include <gtest/gtest.h>
#include "lp_impl.hpp"
#include "lpinterface_gurobi.hpp"

using namespace lpint;

GurobiSolver create_grb() {
  LinearProgram lp(OptimizationType::Maximize, 
                                  {Row<double>({1.0, 1.0, 2.0}, {0, 1, 2}),
                                   Row<double>({1.0, 2.0, 3.0}, {0, 1, 2}),
                                   Row<double>({1, 0, 1.0}, {0, 1})});
  GurobiSolver grb(std::make_shared<LinearProgram>(lp));
  return grb;
}

TEST(Gurobi, SetParameters) {
  auto grb = create_grb();
  auto err = grb.set_parameter(Param::GrbThreads, 1);
  ASSERT_TRUE(err);
  err = grb.set_parameter(Param::GrbOutputFlag, 0);
  ASSERT_TRUE(err);
}

TEST(Gurobi, AddConstraints) { 
  auto grb = create_grb(); 
  
}