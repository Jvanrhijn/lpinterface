#include "lpinterface.hpp"

#ifdef LPINT_GUROBI_SUPPORTED
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#endif

#ifdef LPINT_SOPLEX_SUPPORTED
#include "lpinterface/soplex/lpinterface_soplex.hpp"
#endif

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <rapidcheck.h>

#include "testutil.hpp"
#include "test_common.hpp"

using namespace lpint;

constexpr std::size_t ncols = 10;
constexpr std::size_t nrows = 50;

struct ConstraintProperties {
  template <class Solver>
  static void exec() {
    test_add_retrieve_constraints<Solver>(ncols);
    test_add_remove_constraints<Solver>(ncols);
  }
};

struct TimeLimitProperties {
  template <class Solver>
  static void exec() {
    test_timelimit<Solver>(ncols);
  }
};

struct LinearProgramProperties {
  template <class Solver>
  static void exec() {
    test_num_constraints<Solver>(nrows, ncols);
    test_num_vars<Solver>(nrows, ncols);
  }
};

struct FullProblemTests {
  template <class Solver>
  static void exec() {
    test_full_problem<Solver>();
    test_raw_data_full_problem<Solver>();
  }
};

struct Variables {
  template <class Solver>
  static void exec() {
    test_num_vars<Solver>();
  }
};

TEST(Solvers, Constraints) {
  for_each_type<ConstraintProperties, LPINT_SUPPORTED_SOLVERS>();
}

TEST(Solvers, TimeLimit) {
  for_each_type<TimeLimitProperties, LPINT_SUPPORTED_SOLVERS>();
}

TEST(Solvers, LinearProgram) {
  for_each_type<LinearProgramProperties, LPINT_SUPPORTED_SOLVERS>();
}

TEST(Solvers, FullProblem) {
  for_each_type<FullProblemTests, LPINT_SUPPORTED_SOLVERS>();
}

TEST(Solvers, AddVars) {
  for_each_type<Variables, LPINT_SUPPORTED_SOLVERS>();
}