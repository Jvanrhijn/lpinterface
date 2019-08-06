#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "generators.hpp"
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "mock_lp.hpp"

using namespace lpint;

inline GurobiSolver create_grb(const LinearProgram& lp) {
  GurobiSolver grb(std::make_shared<LinearProgram>(lp));
  return grb;
}

inline int configure_gurobi(LinearProgram& lp, GRBenv **env, GRBmodel **model) {
  int saved_stdout = dup(1);
  close(1);
  int new_stdout = open("/dev/null", O_WRONLY);

  int error = GRBloadenv(env, "");

  close(new_stdout);
  new_stdout = dup(saved_stdout);
  close(saved_stdout);

  if (error) {
    return error;
  }
  error = GRBnewmodel(*env, model, nullptr, 0, nullptr, nullptr, nullptr, nullptr,
                      nullptr);
  if (error) {
    return error;
  }

  GRBsetintparam(GRBgetenv(*model), "outputflag", 0);

  // set objective sense
  error = GRBsetintattr(*model, GRB_INT_ATTR_MODELSENSE,
                        lp.optimization_type() == OptimizationType::Maximize
                            ? GRB_MAXIMIZE
                            : GRB_MINIMIZE);
  if (error) {
    return error;
  }
  // add variables
  error = GRBaddvars(
      *model, lp.num_vars(), 0, nullptr, nullptr, nullptr,
      lp.objective().values.data(), nullptr, nullptr,
      GurobiSolver::convert_variable_type(lp.objective().variable_types).data(),
      nullptr);

  if (error) {
    return error;
  }

  const auto constraints = lp.constraints();

  // add constraints
  std::size_t idx = 0;
  for (auto& row : lp.matrix()) {
    error = GRBaddconstr(
        *model, row.num_nonzero(), row.nonzero_indices().data(), row.values().data(),
        GurobiSolver::convert_ordering(constraints[idx].ordering),
        constraints[idx].value, ("constr" + std::to_string(idx)).c_str());
    if (error) {
      return error;
    }
    idx++;
  }
  return 0;
}

TEST(Gurobi, SetParameters) {
  auto lp = std::make_shared<MockLinearProgram>();
  EXPECT_CALL(*lp.get(), optimization_type()).Times(1);
  GurobiSolver grb(lp);
  grb.set_parameter(Param::GrbThreads, 1);
  grb.set_parameter(Param::GrbOutputFlag, 0);
}

RC_GTEST_PROP(Gurobi, SameResultAsBareGurobi, ()) {
  constexpr double TIME_LIMIT = 10.0;

  auto lp = *rc::genLinearProgram(
      100, 100, rc::gen::element(Ordering::LEQ, Ordering::GEQ, Ordering::EQ),
      rc::gen::arbitrary<VarType>());

  GRBenv* env = nullptr;
  GRBmodel* model = nullptr;

  auto grb = create_grb(lp);

  grb.set_parameter(Param::TimeLimit, TIME_LIMIT);

  int error;
  try {
    error = configure_gurobi(lp, &env, &model);
    GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, TIME_LIMIT);
    grb.update_program();
  } catch (const GurobiException& e) {
    RC_ASSERT(e.code() == error);
    return;
  }

  // solve the lp
  Status status;
  try {
    error = GRBoptimize(model);
    status = grb.solve_primal();
  } catch (const GurobiException& e) {
    std::cout << e.what() << std::endl;
    RC_ASSERT(e.code() == error);
    return; 
  }

  // retrieve solution info from gurobi
  int gurobi_status;
  do {
    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &gurobi_status);
  } while (gurobi_status == GRB_INPROGRESS);

  if (error) {
    throw GurobiException(error, GRBgeterrormsg(env));
  }

  // gurobi and interface should return same status
  RC_ASSERT(GurobiSolver::convert_gurobi_status(gurobi_status) == status);

  if (status == Status::Optimal) {
    std::vector<double> solution(lp.num_vars());
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, lp.num_vars(),
                               solution.data());
    RC_ASSERT(solution == grb.get_solution().primal);

    double objval;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    RC_ASSERT(std::abs(objval - grb.get_solution().objective_value) < 1e-15);
  }

  GRBfreemodel(model);
  GRBfreeenv(env);
}

TEST(Gurobi, FullProblem) {
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
  auto grb = create_grb(lp);
  // Update the internal Gurobi LP
  grb.update_program();

  // Solve the primal LP problem
  auto status = grb.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  // check solution value
  auto solution = grb.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{1.0, 0.0, 1.0}));
  ASSERT_EQ(solution.objective_value, 3.0);
}

TEST(Gurobi, FullProblemRawData) {
  // Create the Gurobi solver
  GurobiSolver grb(OptimizationType::Maximize);

  {
    std::vector<double> values = {1, 2, 3, 1, 1};
    std::vector<int> start_indices = {0, 3};
    std::vector<int> col_indices = {0, 1, 2, 0, 1};
    std::vector<double> rhs = {4.0, 1.0};
    std::vector<Ordering> ord = {Ordering::LEQ, Ordering::GEQ};
    std::vector<double> objective = {1.0, 1.0, 2.0};
    std::vector<VarType> var_type = {VarType::Binary, VarType::Binary,
                                     VarType::Binary};

    grb.add_variables(std::move(objective), std::move(var_type));
    grb.add_rows(std::move(values), std::move(start_indices),
                 std::move(col_indices), std::move(ord), std::move(rhs));
  }

  // Solve the primal LP problem
  auto status = grb.solve_primal();
  ASSERT_EQ(status, Status::Optimal);

  //// check solution value
  auto solution = grb.get_solution();

  ASSERT_EQ(solution.primal, (std::vector<double>{1.0, 0.0, 1.0}));
  ASSERT_EQ(solution.objective_value, 3.0);
}
