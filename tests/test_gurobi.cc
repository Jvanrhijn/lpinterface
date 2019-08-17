#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include "generators.hpp"
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "mock_lp.hpp"

using namespace lpint;
using namespace testing;

constexpr const std::size_t nrows = 100;
constexpr const std::size_t ncols = 100;

inline void redirect_stdout(int *saved_stdout, int *new_stdout) {
  *saved_stdout = dup(1);
  close(1);
  *new_stdout = open("/dev/null", O_WRONLY);
}

inline void restore_stdout(int *saved_stdout, int *new_stdout) {
  close(*new_stdout);
  close(*saved_stdout);
  close(*saved_stdout);
  delete saved_stdout;
  delete new_stdout;
}

inline int configure_gurobi(LinearProgram& lp, GRBenv** env, GRBmodel** model) {
  int saved_stdout = dup(1);
  close(1);
  int new_stdout = open("/dev/null", O_WRONLY);
  int error = GRBloadenv(env, "");
  close(new_stdout);
  dup(saved_stdout);
  close(saved_stdout);

  if (error) {
    return error;
  }
  error = GRBnewmodel(*env, model, nullptr, 0, nullptr, nullptr, nullptr,
                      nullptr, nullptr);
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

  auto& constraints = lp.constraints();

  // add constraints
  std::size_t idx = 0;
  for (auto& constraint : constraints) {
    auto& row = constraint.row;
    error = GRBaddconstr(*model, row.num_nonzero(),
                         row.nonzero_indices().data(), row.values().data(),
                         GurobiSolver::convert_ordering(constraint.ordering),
                         constraint.value,
                         ("constr" + std::to_string(idx)).c_str());
    if (error) {
      return error;
    }
    idx++;
  }
  return 0;
}

TEST(Gurobi, SetParameters) {
  auto lp = std::make_unique<MockLinearProgram>();
  EXPECT_CALL(*lp, optimization_type()).Times(1);
  GurobiSolver grb(std::move(lp));
  grb.set_parameter(Param::Threads, 1);
  grb.set_parameter(Param::Verbosity, 0);
}

TEST(Gurobi, UninitializedLP) {
  auto lp = std::make_unique<NiceMock<MockLinearProgram>>();
  EXPECT_CALL(*lp, is_initialized()).WillOnce(Return(false));
  GurobiSolver grb(std::move(lp));
  EXPECT_THROW(grb.update_program(), LinearProgramNotInitializedException);
}

RC_GTEST_PROP(Gurobi, TimeOutWhenTimeLimitZero, ()) {
  // generate a linear program that is not unbounded or infeasible
  auto lp = gen_simple_valid_lp(nrows, ncols);
  GurobiSolver grb(std::move(lp));
  grb.update_program();
  grb.set_parameter(Param::TimeLimit, 0.0);
  const auto status = grb.solve_primal();
  RC_ASSERT(status == Status::TimeOut);
}

RC_GTEST_PROP(Gurobi, IterationLimit, ()) {
  // generate a linear program that is not unbounded or infeasible,
  // and requires more than 0 iterations
  auto lp = gen_simple_valid_lp(10, ncols, 2.0);
  GurobiSolver grb(std::move(lp));
  grb.set_parameter(Param::IterationLimit, 0.0);
  grb.update_program();
  const auto status = grb.solve_primal();
  RC_ASSERT(status == Status::IterationLimit);
}


TEST(Gurobi, UpdateProgram) {
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

  GurobiSolver grb(std::move(lp));
  grb.update_program();
}

RC_GTEST_PROP(Gurobi, SameResultAsBareGurobi, ()) {
  constexpr double TIME_LIMIT = 0.1;

  auto lp = *rc::genLinearProgramPtr(
      nrows, ncols, rc::gen::element(Ordering::LEQ, Ordering::GEQ, Ordering::EQ),
      rc::gen::arbitrary<VarType>());

  GRBenv* env = nullptr;
  GRBmodel* model = nullptr;

  GurobiSolver grb;

  int error;
  try {
    error = configure_gurobi(*lp, &env, &model);
    GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, TIME_LIMIT);
    grb = GurobiSolver(std::move(lp));
    grb.set_parameter(Param::TimeLimit, TIME_LIMIT);
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
    const auto nvars = grb.linear_program().num_vars();
    std::vector<double> solution(nvars);
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, solution.size(),
                               solution.data());
    RC_ASSERT(solution == grb.get_solution().primal);

    double objval;
    GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    RC_ASSERT(std::abs(objval - grb.get_solution().objective_value) < 1e-15);
  }

  GRBfreemodel(model);
  GRBfreeenv(env);
}

TEST(Gurobi, FullProblem) {
  auto lp = std::make_unique<LinearProgram>(OptimizationType::Maximize);

  std::vector<Constraint<double>> constr;
  constr.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), Ordering::LEQ, 4.0);
  constr.emplace_back(Row<double>({1, 1}, {0, 1}), Ordering::GEQ, 1.0);

  lp->add_constraints(std::move(constr));

  Objective<double> obj{{1.0, 1.0, 2.0},
                        {VarType::Binary, VarType::Binary, VarType::Binary}};
  lp->set_objective(obj);

  // Create the Gurobi solver from the given LP
  GurobiSolver grb(std::move(lp));
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

RC_GTEST_PROP(Gurobi, RawDataSameAsBareGurobi, ()) {
  auto lp_data = generate_lp_data(
      10, 10, rc::gen::element(Ordering::EQ, Ordering::LEQ, Ordering::GEQ),
      rc::gen::arbitrary<VarType>());

  // configure bare Gurobi
  GRBenv* env;
  GRBmodel* model;

  int saved_stdout = dup(1);
  close(1);
  int new_stdout = open("/dev/null", O_WRONLY);

  GRBloadenv(&env, "");

  close(new_stdout);
  dup(saved_stdout);
  close(saved_stdout);

  GRBnewmodel(env, &model, nullptr, 0, nullptr, nullptr, nullptr,
                      nullptr, nullptr);
  GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_OUTPUTFLAG, 0);
 
  GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE,
                lp_data.sense == OptimizationType::Maximize ? GRB_MAXIMIZE
                                                            : GRB_MINIMIZE);

  auto gurobi_var_type = GurobiSolver::convert_variable_type(lp_data.var_type);
  GRBaddvars(model, lp_data.objective.size(), 0, nullptr, nullptr,
                     nullptr, lp_data.objective.data(), nullptr, nullptr,
                     gurobi_var_type.data(), nullptr);

  std::vector<char> gurobi_sense(lp_data.ord.size());
  std::transform(lp_data.ord.begin(), lp_data.ord.end(), gurobi_sense.begin(),
                 GurobiSolver::convert_ordering);

  int error = GRBaddconstrs(model, lp_data.start_indices.size(),
                        lp_data.values.size(), lp_data.start_indices.data(),
                        lp_data.column_indices.data(), lp_data.values.data(),
                        gurobi_sense.data(), lp_data.rhs.data(), nullptr);

  // now configure LP interface
  GurobiSolver grb(lp_data.sense);

  grb.add_variables(std::move(lp_data.objective), std::move(lp_data.var_type));
  grb.add_rows(std::move(lp_data.values), std::move(lp_data.start_indices),
               std::move(lp_data.column_indices), std::move(lp_data.ord),
               std::move(lp_data.rhs));

  try {
    constexpr double TIME_LIMIT = 0.1;
    GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, TIME_LIMIT);
    grb.set_parameter(Param::TimeLimit, TIME_LIMIT);
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
    int nvars;
    GRBgetintattr(model, GRB_INT_ATTR_NUMVARS, &nvars);
    std::vector<double> solution(static_cast<std::size_t>(nvars));
    GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, solution.size(),
                               solution.data());
    RC_ASSERT(solution == grb.get_solution().primal);

    double objval;
    GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    RC_ASSERT(std::abs(objval - grb.get_solution().objective_value) < 1e-15);
  }

  GRBfreemodel(model);
  GRBfreeenv(env);
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
