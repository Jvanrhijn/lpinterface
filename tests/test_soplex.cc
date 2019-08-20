#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <iostream>
#include <memory>

#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"
#include "mock_lp.hpp"

#include "generators.hpp"

using namespace lpint;
using namespace testing;

constexpr const std::size_t nrows = 100;
constexpr const std::size_t ncols = 100;

inline soplex::SoPlex configure_soplex(const LinearProgram& lp) {
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

  const auto& constraints = lp.constraints();

  for (const auto& constraint : constraints) {  // soplex
    DSVector ds_row(constraint.row.values().size());
    ds_row.add(constraint.row.values().size(),
               constraint.row.nonzero_indices().data(),
               constraint.row.values().data());
    soplex.addRowReal(LPRow(constraint.lower_bound, ds_row, constraint.upper_bound));
  }
  return soplex;
}

TEST(Soplex, SetParameters) {
  auto lp = std::make_unique<NiceMock<MockLinearProgram>>();
  EXPECT_CALL(*lp, optimization_type()).Times(1);
  SoplexSolver spl(std::move(lp));
  spl.set_parameter(Param::Verbosity, 0);
}

TEST(Soplex, UpdateProgram) {
  auto lp = std::make_unique<NiceMock<MockLinearProgram>>();
  ON_CALL(*lp, is_initialized()).WillByDefault(Return(true));

  std::vector<Constraint<double>> empty_constr;
  ON_CALL(*lp, constraints()).WillByDefault(ReturnPointee(&empty_constr));

  Objective<double> empty_obj;
  ON_CALL(*lp, objective()).WillByDefault(ReturnPointee(&empty_obj));

  EXPECT_CALL(*lp, is_initialized()).Times(1);
  EXPECT_CALL(*lp, objective()).Times(AtLeast(1));
  EXPECT_CALL(*lp, constraints()).Times(AtLeast(1));
  EXPECT_CALL(*lp, optimization_type()).Times(AtLeast(1));

  SoplexSolver spl(std::move(lp));
  spl.update_program();
}

TEST(Soplex, UninitializedLP) {
  auto lp = std::make_unique<NiceMock<MockLinearProgram>>();
  EXPECT_CALL(*lp, is_initialized()).WillOnce(Return(false));
  SoplexSolver spl(std::move(lp));
  EXPECT_THROW(spl.update_program(), LinearProgramNotInitializedException);
}

RC_GTEST_PROP(Soplex, TimeOutWhenTimeLimitZero, ()) {
  // generate a linear program that is not unbounded or infeasible
  auto lp = gen_simple_valid_lp(1, ncols);
  SoplexSolver soplex(std::move(lp));
  soplex.set_parameter(Param::TimeLimit, 0.0);
  soplex.update_program();
  const auto status = soplex.solve_primal();
  RC_ASSERT(status == Status::TimeOut);
}

RC_GTEST_PROP(Soplex, IterationLimit, ()) {
  // generate a linear program that is not unbounded or infeasible
  auto lp = gen_simple_valid_lp(1, ncols);
  SoplexSolver soplex(std::move(lp));
  soplex.set_parameter(Param::IterationLimit, 0);
  soplex.update_program();
  const auto status = soplex.solve_primal();
  RC_ASSERT(status == Status::IterationLimit);
}

// property: any LP should result in the same
// answer as SoPlex gives us
RC_GTEST_PROP(Soplex, SameResultAsBareSoplex, ()) {
  using namespace soplex;

  auto lp = *rc::genLinearProgramPtr(
      nrows, ncols, rc::gen::element(Ordering::LEQ, Ordering::GEQ),
      rc::gen::just(VarType::Real));

  auto soplex = configure_soplex(*lp);

  SoplexSolver solver(std::move(lp));

  solver.update_program();

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
  auto lp = std::make_unique<LinearProgram>(OptimizationType::Maximize);

  std::vector<Constraint<double>> constr;
  constr.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 4.0);
  constr.emplace_back(Row<double>({1, 1}, {0, 1}), 1.0, LPINT_INFINITY);

  lp->add_constraints(std::move(constr));

  Objective<double> obj({1.0, 1.0, 2.0});
  lp->set_objective(obj);

  // Create the solver from the given LP
  SoplexSolver spl(std::move(lp));

  // Update the internal Gurobi LP
  spl.update_program();

  // // Solve the primal LP problem
  auto status = spl.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  // // check solution value
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
