#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <iostream>
#include <memory>

#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"

#include "generators.hpp"

using namespace lpint;
using namespace testing;

constexpr const std::size_t nrows = 100;
constexpr const std::size_t ncols = 100;

inline soplex::SoPlex configure_soplex(const ILinearProgramHandle& lp) {
  using namespace soplex;

  // intialize soplex
  SoPlex soplex;
  soplex.setIntParam(SoPlex::OBJSENSE,
                     lp.optimization_type() == OptimizationType::Maximize
                         ? SoPlex::OBJSENSE_MAXIMIZE
                         : SoPlex::OBJSENSE_MINIMIZE);
  soplex.setIntParam(SoPlex::VERBOSITY, 0);

  // soplex: add vars
  DSVector dummycol(0);
  for (const auto& coefficient : lp.objective().values) {
    soplex.addColReal(LPCol(coefficient, dummycol, infinity, 0.0));
  }

  for (const auto& constraint : lp.constraints()) {  // soplex
    DSVector ds_row(constraint.row.values().size());
    ds_row.add(constraint.row.values().size(),
               constraint.row.nonzero_indices().data(),
               constraint.row.values().data());
    soplex.addRowReal(LPRow(constraint.lower_bound, ds_row, constraint.upper_bound));
  }
  return soplex;
}

RC_GTEST_PROP(SoPlex, NumConstraints, ()) {
  auto soplex = rc::genLinearProgramSolver<SoplexSolver>(nrows, ncols, 
                                                         rc::gen::just(VarType::Real));
  soplex.set_parameter(Param::Verbosity, 0);
  RC_ASSERT(soplex.linear_program().num_constraints() 
      == soplex.linear_program().constraints().size());
}

RC_GTEST_PROP(SoPlex, NumVars, ()) {
  auto soplex = rc::genLinearProgramSolver<SoplexSolver>(nrows, ncols, 
                                                         rc::gen::just(VarType::Real));
  soplex.set_parameter(Param::Verbosity, 0);
  RC_ASSERT(soplex.linear_program().num_vars() 
      == soplex.linear_program().objective().values.size());
}

RC_GTEST_PROP(Soplex, TimeOutWhenTimeLimitZero, ()) {
  // generate a linear program that is not unbounded or infeasible
  auto soplex = gen_simple_valid_lp<SoplexSolver>(1, ncols);
  soplex.set_parameter(::Param::Verbosity, 0);
  soplex.set_parameter(Param::TimeLimit, 0.0);
  const auto status = soplex.solve_primal();
  RC_ASSERT(status == Status::TimeOut);
}

RC_GTEST_PROP(Soplex, IterationLimit, ()) {
  // generate a linear program that is not unbounded or infeasible
  auto soplex = gen_simple_valid_lp<SoplexSolver>(1, ncols);
  soplex.set_parameter(::Param::Verbosity, 0);
  soplex.set_parameter(Param::IterationLimit, 0);
  const auto status = soplex.solve_primal();
  RC_ASSERT(status == Status::IterationLimit);
}

RC_GTEST_PROP(Soplex, SupportedParams, ()) {
  SoplexSolver soplex;
  RC_ASSERT(!soplex.parameter_supported(Param::Threads));
  RC_ASSERT(soplex.parameter_supported(Param::TimeLimit));
  RC_ASSERT(soplex.parameter_supported(Param::Verbosity));
  RC_ASSERT(soplex.parameter_supported(Param::IterationLimit));
  RC_ASSERT(soplex.parameter_supported(Param::ObjectiveSense));
  RC_ASSERT(soplex.parameter_supported(Param::ObjectiveLowerLimit));
  RC_ASSERT(soplex.parameter_supported(Param::ObjectiveUpperLimit));
  RC_ASSERT(soplex.parameter_supported(Param::Infinity));
}

// property: any LP should result in the same
// answer as SoPlex gives us
RC_GTEST_PROP(Soplex, SameResultAsBareSoplex, ()) {
  using namespace soplex;

  auto solver = rc::genLinearProgramSolver<SoplexSolver>(
      nrows, ncols, 
      rc::gen::just(VarType::Real));

  auto soplex = configure_soplex(solver.linear_program());

  solver.set_parameter(::Param::Verbosity, 0);

  Status status = solver.solve_primal();

  // now we repeat the computation using SoPlex itself
  const auto status_soplex = SoplexSolver::translate_status(soplex.optimize());

  // check whether bare soplex has the same result as our solver
  RC_ASSERT(status_soplex == status);

  // if the optimization was a success, check for equality of solutions
  if (status == Status::Optimal) {
    DVector prim(soplex.numColsReal());
    soplex.getPrimalReal(prim);
    std::vector<double> primal(prim.get_ptr(), prim.get_ptr() + prim.dim());

    DVector dual(soplex.numRowsReal());
    soplex.getDualReal(dual);
    std::vector<double> d(dual.get_ptr(), dual.get_ptr() + dual.dim());

    RC_ASSERT(soplex.objValueReal() == solver.get_solution().objective_value);

    RC_ASSERT(d == solver.get_solution().dual);
  }
}

TEST(Soplex, FullProblem) {
  SoplexSolver spl(OptimizationType::Maximize);

  spl.linear_program().set_objective_sense(OptimizationType::Maximize);

  spl.linear_program().set_objective(Objective<double>({1, 1, 2}));

  std::vector<Constraint<double>> constr;
  constr.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 4.0);
  constr.emplace_back(Row<double>({1, 1}, {0, 1}), 1.0, LPINT_INFINITY);

  spl.linear_program().add_constraints(std::move(constr));

  // // Solve the primal LP problem
  auto status = spl.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  //// // check solution value
  auto solution = spl.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
  ASSERT_NEAR(solution.objective_value, 4.0, 1e-15);
}

//RC_GTEST_PROP(Soplex, RawDataSameAsBareSoplex, ()) {
//  using namespace soplex;
//
//  const auto sense =
//      *rc::gen::arbitrary<OptimizationType>().as("Objective sense");
//
//  std::vector<double> values, objective, rhs;
//  std::vector<int> start_indices, col_indices;
//  std::vector<VarType> var_type;
//  std::vector<Ordering> ord;
//
//  std::tie(values, start_indices, col_indices, rhs, ord, objective, var_type)
//  = generate_lp_data(100, 10);
//
//  SoPlex soplex;
//
//  soplex.
//
//}

TEST(Soplex, FullProblemRawData) {
  // Create the Gurobi solver
  SoplexSolver spl(OptimizationType::Maximize);

  {
    std::vector<double> values = {1, 2, 3, 1, 1};
    std::vector<int> start_indices = {0, 3};
    std::vector<int> col_indices = {0, 1, 2, 0, 1};
    std::vector<double> lb = {-LPINT_INFINITY, 1.0};
    std::vector<double> ub = {4.0, LPINT_INFINITY};
    std::vector<double> objective = {1.0, 1.0, 2.0};
    std::vector<VarType> var_type = {VarType::Real, VarType::Real,
                                     VarType::Real};

    spl.add_variables(std::move(objective), std::move(var_type));
    spl.add_rows(std::move(values), std::move(start_indices),
                 std::move(col_indices), std::move(lb), std::move(ub));
  }

  // Solve the primal LP problem
  auto status = spl.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  //// check solution value
  auto solution = spl.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
  ASSERT_EQ(solution.objective_value, 4.0);
}
