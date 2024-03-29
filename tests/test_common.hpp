#include <gtest/gtest.h>
#include <rapidcheck.h>

#include "generators.hpp"
#include "testutil.hpp"

#include "lpinterface.hpp"

namespace lpint {

template <class Solver>
void test_timelimit(std::size_t ncols) {
  templated_prop<Solver>("Timeout when timelimit is zero", [=]() {
    auto solver = gen_simple_valid_lp<Solver>(1, ncols);
    solver.set_parameter(lpint::Param::Verbosity, 0);
    solver.set_parameter(lpint::Param::TimeLimit, 0.0);
    const auto status = solver.solve();
    RC_ASSERT(status == lpint::Status::TimeOut);
  });
}

template <class Solver>
void test_iterlimit(std::size_t ncols) {
  templated_prop<Solver>("Iteration limit", [=]() {
    // generate a linear program that is not unbounded or infeasible
    auto solver = gen_simple_valid_lp<Solver>(1, ncols);
    solver.set_parameter(Param::Verbosity, 0);
    solver.set_parameter(Param::IterationLimit, 0);
    const auto status = solver.solve();
    RC_ASSERT(status == Status::IterationLimit);
  });
}

template <class Solver>
void test_full_problem() {
  Solver solver(OptimizationType::Maximize);

  solver.set_parameter(Param::Verbosity, 0);

  solver.linear_program().set_objective_sense(OptimizationType::Maximize);

  solver.linear_program().add_variables(3);
  solver.linear_program().set_objective(Objective<double>({1, 1, 2}));

  std::vector<Constraint<double>> constr;
  constr.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 4.0);
  constr.emplace_back(Row<double>({1, 1}, {0, 1}), 1.0, LPINT_INFINITY);

  solver.linear_program().add_constraints(std::move(constr));

  // // Solve the primal LP problem
  auto status = solver.solve();
  ASSERT_EQ(status, Status::Optimal);

  //// // check solution value
  auto solution = solver.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{4.0, 0.0, 0.0}));
  ASSERT_NEAR(solution.objective_value, 4.0, 1e-15);
}

template <class Solver>
void test_add_retrieve_constraints(std::size_t ncols) {
  templated_prop<Solver>("Retrieved constraints are equal to those added", [=]() {
    auto nconstr = *rc::gen::inRange<std::size_t>(1, ncols);
    auto constraints= *rc::gen::container<std::vector<Constraint<double>>>(
      nconstr, 
      rc::genConstraint(
        rc::genRow(
          ncols, 
          rc::gen::nonZero<double>()), 
        rc::gen::arbitrary<double>()));
    std::vector<Constraint<double>> constraints_backup(nconstr);
    std::transform(constraints.begin(), constraints.end(), constraints_backup.begin(),
      [](const Constraint<double>& c) { return copy_constraint<double>(c); } );
    Solver solver(OptimizationType::Maximize);
    // first need to create variables or gurobi will throw
    auto obj = *rc::genSizedObjective(ncols, rc::gen::arbitrary<double>());
    solver.linear_program().add_variables(obj.values.size());
    solver.linear_program().set_objective(std::move(obj));
    solver.linear_program().add_constraints(std::move(constraints));
    auto retrieved_constraints = solver.linear_program().constraints();
    RC_ASSERT(constraints_backup == retrieved_constraints);
  });
}

template <class Solver>
void test_add_remove_constraints(std::size_t ncols) {
  templated_prop<Solver>("Adding and removing constraints works properly", [=]() {
    // generate and backup constraints
    auto nconstr = *rc::gen::inRange<std::size_t>(1, ncols);
    auto constraints= *rc::gen::container<std::vector<Constraint<double>>>(
      nconstr, 
      rc::genConstraint(
        rc::genRow(
          ncols, 
          rc::gen::nonZero<double>()), 
        rc::gen::arbitrary<double>()));
    std::vector<Constraint<double>> constraints_backup(nconstr);
    std::transform(constraints.begin(), constraints.end(), constraints_backup.begin(),
      [](const Constraint<double>& c) { return copy_constraint<double>(c); } );

    const auto nconstr_to_remove = *rc::gen::inRange(1ul, nconstr+1).as("Num constraints to remove");
  
    Solver solver(OptimizationType::Maximize);
    // generate variables to avoid error
    auto obj = *rc::genSizedObjective(ncols, rc::gen::arbitrary<double>());
    solver.linear_program().add_variables(obj.values.size());
    solver.linear_program().set_objective(std::move(obj));
    solver.linear_program().add_constraints(std::move(constraints));

    for (std::size_t i = 0; i < nconstr_to_remove; i++) {
      // generate index of constraint to remove
      auto nconstr_left = solver.linear_program().num_constraints();
      // TODO: generalize to any removal order
      const auto remove_idx = *rc::gen::inRange(0ul, nconstr_left).as("Removal index");

      // remove constraint from backup
      constraints_backup.erase(constraints_backup.begin() 
        + static_cast<std::vector<Constraint<double>>::difference_type>(remove_idx));

      solver.linear_program().remove_constraint(remove_idx);
    }

    RC_ASSERT(solver.linear_program().constraints() == constraints_backup);
  
  });
}

template <class Solver>
void test_num_constraints(std::size_t nrows, std::size_t ncols) {
  templated_prop<Solver>("Number of constraints properly retrieved", [=]() {
    auto solver = rc::genLinearProgramSolver<Solver>(nrows, ncols);
    solver.set_parameter(Param::Verbosity, 0);
    RC_ASSERT(solver.linear_program().num_constraints() 
        == solver.linear_program().constraints().size());
    });
}

template <class Solver>
void test_num_vars(std::size_t nrows, std::size_t ncols) {
  templated_prop<Solver>("Number of variables properly retrieved", [=]() {
    auto solver = rc::genLinearProgramSolver<Solver>(nrows, ncols);
    solver.set_parameter(Param::Verbosity, 0);
    RC_ASSERT(solver.linear_program().num_vars() 
        == solver.linear_program().objective().values.size());
    });
}

template <class Solver>
void test_add_retrieve_objective(std::size_t ncols) {
  templated_prop<Solver>("Test adding and retrieving objective", [=]() {
    auto count = *rc::gen::inRange<std::size_t>(0, ncols);
    auto obj = *rc::genSizedObjective(ncols, rc::gen::arbitrary<double>());

    std::vector<double> vals = obj.values;
    Objective<double> obj_backup(std::move(vals));

    Solver solver(*rc::gen::arbitrary<OptimizationType>());
    solver.linear_program().add_variables(obj.values.size());
    solver.linear_program().set_objective(std::move(obj));
    RC_ASSERT(obj_backup == solver.linear_program().objective());
  });
}

template <class Solver>
void test_supported_params(std::initializer_list<Param> supported, 
                           std::initializer_list<Param> not_supported) {
  Solver solver;
  for (const auto& param : supported) {
    ASSERT_TRUE(solver.parameter_supported(param));
  }
  for (const auto& param: not_supported) {
    ASSERT_TRUE(!solver.parameter_supported(param));
    RC_ASSERT_THROWS_AS(solver.set_parameter(param, 0), 
                        UnsupportedParameterException);
  }
}

template <class Solver>
void test_model_not_solved_acces_throw() {
  Solver solver;
  RC_ASSERT_THROWS_AS(solver.get_solution(), ModelNotSolvedException);
}

template <class Solver>
void test_num_vars() {
  templated_prop<Solver>("Adding variables causes correct num_vars() return values", [=]() {
    auto nvars = *rc::gen::inRange<std::size_t>(0, 10000);
    Solver solver;
    solver.linear_program().add_variables(nvars);
    RC_ASSERT(solver.linear_program().num_vars() == nvars);
    const auto vars = *rc::gen::container<std::vector<Variable>>(rc::gen::arbitrary<Variable>());
    solver.linear_program().add_variables(vars);
    RC_ASSERT(solver.linear_program().num_vars() == nvars + vars.size());
  });
}

template <class Solver>
void test_add_retrieve_vars() {
  templated_prop<Solver>("Retrieved variables from LP are equal to inserted variables", [=]() {
    auto vars = *rc::gen::container<std::vector<Variable>>(rc::gen::arbitrary<Variable>());
    Solver solver;
    solver.linear_program().add_variables(vars);
    RC_ASSERT(vars == solver.linear_program().variables());
  });
}

template <class Solver>
void test_add_remove_vars() {
  templated_prop<Solver>("Removing variables from LP preserves ordering", [=]() {
    auto vars = *rc::gen::container<std::vector<Variable>>(rc::gen::arbitrary<Variable>())
      .as("Variables");
    Solver solver;
    solver.linear_program().add_variables(vars);

    auto nvars_to_remove = *rc::gen::inRange<std::size_t>(1, vars.size()+1).as("Num of vars to remove");

    for (std::size_t i = 0; i < nvars_to_remove; i++) {
      auto nvars_left = vars.size();
      auto to_remove = *rc::gen::inRange<std::size_t>(0, nvars_left).as("Removal index");

      solver.linear_program().remove_variable(to_remove);
      
      vars.erase(vars.begin() + static_cast<decltype(vars)::difference_type>(to_remove));
    }

    RC_ASSERT(vars == solver.linear_program().variables());
  });
}


} // namespace lpint