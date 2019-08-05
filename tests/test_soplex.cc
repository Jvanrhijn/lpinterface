#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <iostream>

#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"
#include "mock_lp.hpp"

#include "generators.hpp"

using namespace lpint;

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

  const auto constraints = lp.constraints();

  std::size_t i = 0;
  for (const auto& row : lp.matrix()) {  // soplex
    DSVector ds_row(row.values().size());
    ds_row.add(row.values().size(),
               std::vector<int>(row.nonzero_indices().begin(),
                                row.nonzero_indices().end())
                   .data(),
               row.values().data());
    if (lp.constraints()[i].ordering == Ordering::LEQ) {
      soplex.addRowReal(LPRow(0, ds_row, constraints[i].value));
    } else if (lp.constraints()[i].ordering == Ordering::GEQ) {
      soplex.addRowReal(LPRow(constraints[i].value, ds_row, infinity));
    } else {
      throw UnsupportedConstraintException();
    }
    i++;
  }
  return soplex;
}

inline SoplexSolver create_spl(const LinearProgram& lp) {
  SoplexSolver spl(std::make_shared<LinearProgram>(lp));
  return spl;
}

TEST(Soplex, SetParameters) {
  auto lp = std::make_shared<MockLinearProgram>();
  EXPECT_CALL(*lp.get(), optimization_type()).Times(1);
  SoplexSolver spl(lp);
  spl.set_parameter(Param::Verbosity, 0);
}

// TEST(Soplex, UpdateProgram) {
//  auto lp = std::make_shared<MockLinearProgram>();
//  EXPECT_CALL(*lp.get(), matrix()).Times(testing::AtLeast(1));
//  EXPECT_CALL(*lp.get(), optimization_type()).Times(testing::AtLeast(1));
//  SoplexSolver spl(lp);
//  spl.update_program();
//}

// property: any LP should result in the same
// answer as SoPlex gives us
RC_GTEST_PROP(Soplex, SameResultAsBareSoplex, ()) {
  using namespace soplex;

  auto lp = *rc::genLinearProgram(
      100, 100, rc::gen::element(Ordering::LEQ, Ordering::GEQ),
      rc::gen::just(VarType::Real));

  SoplexSolver solver(std::make_shared<LinearProgram>(lp));

  solver.update_program();

  Status status = solver.solve_primal();

  // now we repeat the computation using SoPlex itself
  auto soplex = configure_soplex(lp);
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
  LinearProgram lp(
      OptimizationType::Maximize,
      {Row<double>({1, 2, 3}, {0, 1, 2}), Row<double>({1, 1}, {0, 1})});

  std::vector<Constraint<double>> constr = {
      Constraint<double>{Ordering::LEQ, 4.0},
      Constraint<double>{Ordering::GEQ, 1.0}};

  lp.add_constraints(constr);

  Objective<double> obj{{1.0, 1.0, 2.0},
                        {VarType::Real, VarType::Real, VarType::Real}};
  lp.set_objective(obj);

  // Create the solver from the given LP
  auto spl = create_spl(lp);

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

TEST(Soplex, FullProblemRawData) {
  // Create the Gurobi solver
  SoplexSolver spl(OptimizationType::Maximize);

  {
    std::vector<double> values = {1, 2, 3, 1, 1};
    std::vector<int> start_indices = {0, 3};
    std::vector<int> col_indices = {0, 1, 2, 0, 1};
    std::vector<double> rhs = {4.0, 1.0};
    std::vector<Ordering> ord = {Ordering::LEQ, Ordering::GEQ};
    std::vector<double> objective = {1.0, 1.0, 2.0};
    std::vector<VarType> var_type = {VarType::Real, VarType::Real,
                                     VarType::Real};

    spl.add_variables(std::move(objective), std::move(var_type));
    spl.add_rows(std::move(values), std::move(start_indices),
                 std::move(col_indices), std::move(ord), std::move(rhs));
  }

  // Solve the primal LP problem
  auto status = spl.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  //// check solution value
  auto solution = spl.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
  ASSERT_EQ(solution.objective_value, 4.0);
}
