#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <iostream>
#include <memory>

#include "lpinterface.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"

#include "generators.hpp"
#include "testutil.hpp"
#include "test_common.hpp"

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

TEST(SoPlex, NumConstraints) {
  test_num_constraints<SoplexSolver>(nrows, ncols);
}

TEST(SoPlex, NumVars) {
  test_num_vars<SoplexSolver>(nrows, ncols);
}

TEST(SoPlex, AddAndRetrieveObjective) {
  //test_add_retrieve_objective<SoplexSolver>(ncols, rc::gen::just(VarType::Real));
}

TEST(SoPlex, AddAndRetrieveConstraints) {
  test_add_retrieve_constraints<SoplexSolver>(ncols);
}

TEST(SoPlex, AddAndRemoveConstraints) {
  test_add_remove_constraints<SoplexSolver>(ncols);
}

TEST(Soplex, TimeOutWhenTimeLimitZero) {
  test_timelimit<SoplexSolver>(ncols);
}

TEST(Soplex, IterationLimit) {
  test_iterlimit<SoplexSolver>(ncols);
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
  test_full_problem<SoplexSolver>();
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
  test_raw_data_full_problem<SoplexSolver>();
}
