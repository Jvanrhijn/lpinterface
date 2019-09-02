#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include "generators.hpp"
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "testutil.hpp"
#include "test_common.hpp"
  
using namespace lpint;
using namespace testing;

constexpr const std::size_t nrows = 50;
constexpr const std::size_t ncols = 50;

inline int configure_gurobi(const ILinearProgramHandle& lp, GRBenv** env, GRBmodel** model) {
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
  auto obj = lp.objective();
  error = GRBaddvars(
      *model, obj.values.size(), 0, nullptr, nullptr, nullptr,
      obj.values.data(), nullptr, nullptr,
      LinearProgramHandleGurobi::convert_variable_type(obj.variable_types).data(),
      nullptr);

  if (error) {
    return error;
  }

  auto constraints = lp.constraints();

  // add constraints
  std::size_t idx = 0;
  for (auto& constraint : constraints) {
    auto& row = constraint.row;
    error = GRBaddrangeconstr(*model, row.num_nonzero(),
                         row.nonzero_indices().data(), row.values().data(),
                         constraint.lower_bound,
                         constraint.upper_bound,
                         nullptr);
    if (error) {
      return error;
    }
    idx++;
  }
  return 0;
}


TEST(Gurobi, AddAndRetrieveConstraints) {
  test_add_retrieve_constraints<GurobiSolver>(ncols);
}

TEST(Gurobi, AddAndRemoveConstraints) {
  test_add_remove_constraints<GurobiSolver>(ncols);
}

RC_GTEST_PROP(Gurobi, AddAndRetrieveObjective, ()) {
  auto count = *rc::gen::inRange<std::size_t>(0, ncols);
  auto obj = *rc::genSizedObjective(ncols, rc::gen::arbitrary<VarType>(), rc::gen::arbitrary<double>());

  std::vector<double> vals = obj.values;
  std::vector<VarType> vts = obj.variable_types;
  Objective<double> obj_backup(std::move(vals), std::move(vts));

  GurobiSolver grb(*rc::gen::arbitrary<OptimizationType>());
  grb.linear_program().set_objective(std::move(obj));
  RC_ASSERT(obj_backup == grb.linear_program().objective());
}

TEST(Gurobi, NumConstraints) {
  test_num_constraints<GurobiSolver>(nrows, ncols);
}

TEST(Gurobi, NumVars) {
  test_num_vars<GurobiSolver>(nrows, ncols);
}

TEST(Gurobi, TimeOutWhenTimeLimitZero) {
  test_timelimit<GurobiSolver>(ncols);
}

//RC_GTEST_PROP(Gurobi, IterationLimit, ()) {
//  // generate a linear program that is not unbounded or infeasible,
//  // and requires more than 0 iterations
//  auto lp = gen_simple_valid_lp<LinearProgramHandleGurobi>(10, ncols, 2.0);
//  GurobiSolver grb(std::move(lp));
//  grb.set_parameter(Param::IterationLimit, 0.0);
//  const auto status = grb.solve_primal();
//  RC_ASSERT(status == Status::IterationLimit);
//}

RC_GTEST_PROP(Gurobi, SupportedParams, ()) {
  GurobiSolver grb;
  RC_ASSERT(grb.parameter_supported(Param::Threads));
  RC_ASSERT(grb.parameter_supported(Param::TimeLimit));
  RC_ASSERT(grb.parameter_supported(Param::Verbosity));
  RC_ASSERT(grb.parameter_supported(Param::IterationLimit));
}

RC_GTEST_PROP(Gurobi, SameResultAsBareGurobi, ()) {
  constexpr double TIME_LIMIT = 0.1;

  auto grb = rc::genLinearProgramSolver<GurobiSolver>(
      nrows, ncols, 
      rc::gen::arbitrary<VarType>());

  GRBenv* env = nullptr;
  GRBmodel* model = nullptr;

  int error;
  try {
    error = configure_gurobi(grb.linear_program(), &env, &model);
    GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, TIME_LIMIT);
    grb.set_parameter(Param::TimeLimit, TIME_LIMIT);
    grb.set_parameter(Param::Verbosity, 0);
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

    auto nconstr = grb.linear_program().constraints().size();
    std::vector<double> dual(nconstr);
    int err = GRBgetdblattrarray(model, GRB_DBL_ATTR_PI, 0, dual.size(), 
                                dual.data());
    if (err != 10005) {
      RC_ASSERT(dual == grb.get_solution().dual);
    }

    double objval;
    GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    RC_ASSERT(std::abs(objval - grb.get_solution().objective_value) < 1e-15);
  }

  GRBfreemodel(model);
  GRBfreeenv(env);
}

TEST(Gurobi, FullProblem) {
  test_full_problem<GurobiSolver>();
}

RC_GTEST_PROP(Gurobi, RawDataSameAsBareGurobi, ()) {
  auto lp_data = generate_lp_data<GurobiSolver>(
      10, 10,
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

  auto gurobi_var_type = LinearProgramHandleGurobi::convert_variable_type(lp_data.var_type);
  GRBaddvars(model, lp_data.objective.size(), 0, nullptr, nullptr,
                     nullptr, lp_data.objective.data(), nullptr, nullptr,
                     gurobi_var_type.data(), nullptr);

  int error = GRBaddrangeconstrs(model, lp_data.start_indices.size(),
                        lp_data.values.size(), lp_data.start_indices.data(),
                        lp_data.column_indices.data(), lp_data.values.data(),
                        lp_data.lb.data(), lp_data.ub.data(), nullptr);

  // now configure LP interface
  GurobiSolver grb(lp_data.sense);
  grb.set_parameter(Param::Verbosity, 0);

  grb.add_variables(std::move(lp_data.objective), std::move(lp_data.var_type));
  grb.add_rows(std::move(lp_data.values), std::move(lp_data.start_indices),
               std::move(lp_data.column_indices), std::move(lp_data.lb),
               std::move(lp_data.ub));

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
    auto nvars = grb.linear_program().num_vars();
    std::vector<double> solution(nvars);
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
  test_raw_data_full_problem<GurobiSolver>();
}
