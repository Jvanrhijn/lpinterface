#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <iostream>

#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"
#include "mock_lp.hpp"

#include "generators.hpp"

using namespace lpint;

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

// property: any LP should result in the same
// answer as SoPlex gives us
RC_GTEST_PROP(Soplex, TestGen, ()) {
  using namespace soplex;
  // intialize soplex
  SoPlex soplex;
  soplex.setIntParam(SoPlex::OBJSENSE, SoPlex::OBJSENSE_MAXIMIZE);
  soplex.setIntParam(SoPlex::VERBOSITY, 0);

  // construct an LP
  LinearProgram lp(OptimizationType::Maximize, SparseMatrixType::RowWise);

  // TODO: generalize
  const std::size_t nrows = *rc::gen::inRange<std::size_t>(1, 100);
  const std::size_t ncols = *rc::gen::inRange<std::size_t>(1, 100);

  // generate objective
  const auto objective = *rc::genSizedObjective(
      ncols, rc::gen::just(VarType::Real), rc::gen::arbitrary<double>());
  lp.set_objective(objective);

  // soplex: add vars
  DSVector dummycol(0);
  for (const auto& coefficient : objective.values) {
    soplex.addColReal(LPCol(coefficient, dummycol, infinity, 0.0));
  }

  // generate constraints
  auto constraints = *rc::gen::container<std::vector<Constraint<double>>>(
      nrows, rc::genConstraintWithOrdering(
                 rc::gen::arbitrary<double>(),
                 rc::gen::element(Ordering::LEQ, Ordering::GEQ)));

  // generate constraint matrix
  std::vector<Row<double>> rows;
  for (std::size_t i = 0; i < nrows; i++) {
    auto values = *rc::gen::container<std::vector<double>>(
                       ncols, rc::gen::arbitrary<double>())
                       .as("Row values");
    auto indices = *rc::gen::uniqueCount<std::vector<std::size_t>>(
                        ncols, rc::gen::inRange(0ul, values.size()))
                        .as("Row indices");
    rows.emplace_back(values, indices);

    // soplex
    DSVector ds_row(values.size());
    ds_row.add(values.size(), std::vector<int>(indices.begin(), indices.end()).data(), values.data());
    if (constraints[i].ordering == Ordering::LEQ) {
      soplex.addRowReal(LPRow(0, ds_row, constraints[i].value));
    } else {
      soplex.addRowReal(LPRow(constraints[i].value, ds_row, infinity));
    }
  }

  lp.add_rows(std::move(rows));
  lp.add_constraints(std::move(constraints));

  SoplexSolver solver(std::make_shared<LinearProgram>(lp));

  solver.update_program();

  Status status = solver.solve_primal();

  // now we repeat the computation using SoPlex itsel
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

  // Create the Gurobi solver from the given LP
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
