# lpinterface

[![Build Status](https://travis-ci.org/Jvanrhijn/lpinterface.svg?branch=master)](https://travis-ci.org/Jvanrhijn/lpinterface)
[![codecov](https://codecov.io/gh/Jvanrhijn/lpinterface/branch/master/graph/badge.svg)](https://codecov.io/gh/Jvanrhijn/lpinterface)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/778d6d1fcbf24b63bb084c22aa3b00d4)](https://www.codacy.com/app/Jvanrhijn/lpinterface?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Jvanrhijn/lpinterface&amp;utm_campaign=Badge_Grade)
[![docs](https://img.shields.io/badge/docs-gh--pages-blue)](https://jvanrhijn.github.io/lpinterface/)
[![license](https://img.shields.io/badge/license-MIT-green)]()
[![stc](https://img.shields.io/badge/std-C%2B%2B11-green)]()

Common interface for different linear programming and (mixed-)integer programming solvers.

This library uses a polymorphic interface in order to allow seamless interchange of
linear programming solvers in application code.

## Supported solver backends

* Gurobi
* SoPlex

## Supported compilers

The code is tested in Travis against:

* GCC 6.0, 7.0, 8.0
* Clang 3.6, 3.7, 4.0, 5.0, 6.0

## Example usage

In your CMakeLists.txt: `add_subdirectory(path_to_lpinterface)`. Then simply
link to the library:

~~~cmake
target_link_libraries(${TARGET} lpinterface)
~~~

~~~cpp
#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

#include <memory>
#include <iostream>

using namespace lpint;

struct Wrapper {
  std::unique_ptr<LinearProgramSolver> solver;
  Wrapper(std::unique_ptr<LinearProgramSolver>&& s) : solver(std::move(s)) {}
};

template <typename Solver>
Wrapper create_solver() {
  return Wrapper(std::make_unique<Solver>());
}

int main() {
  // Create the polymorphic solver backend.
  // Switch out the type parameter to change
  // solver backends.
  auto wrapper = create_solver<GurobiSolver>();

  // Retrieve a handle to the linear program from
  // the solver
  auto& lp = wrapper.solver->linear_program();
  
  lp.set_objective_sense(OptimizationType::Maximize);

  // Set the objective function; see the documentation for a list of
  // supported variable types per solver backend.
  // NOTE: the objective function should be set before
  // adding constraints, so the solver knows how many variables
  // there are in the program.
  lp.set_objective(Objective<double>{{1, 2, 3, 4}});

  // Add constraints; these represent the constraint equations.
  // These constraints are equivalent to the equations
  // -inf <= x + 3y + 3z      <= 1
  // -inf <=     4y + 5z + 6w <= 4
  // The Row<T> objects are sparse matrix rows in CSR format.
  std::vector<Constraint<double>> constraints;
  constraints.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 1.0);
  constraints.emplace_back(Row<double>({4, 5, 6}, {1, 2, 3}), -LPINT_INFINITY, 4.0);
  lp.add_constraints(constraints);

  // set solver parameters, see Param documentation for all
  // possible parameter settings.
  wrapper.solver->set_parameter(Param::TimeLimit, 10.0); // max 10 seconds duration
  wrapper.solver->set_parameter(Param::Verbosity, 0); // don't output anything
  wrapper.solver->set_parameter(Param::PrimalOrDual, 0); // use primal simplex
  
  // solve the LP.
  const Status status = wrapper.solver->solve();

  // check the solution status
  switch (status) {
    case (Status::Optimal):
      std::cout << "Optimal objective: " 
                <<  wrapper.solver->get_solution().objective_value 
                << std::endl;
      const std::vector<double> primal = wrapper.solver->get_solution().primal;
      const std::vector<double> dual = wrapper.solver->get_solution().dual;
      // do something with the primal/dual solution
      break;
    case (Status::Unbounded):
      std::cerr << "LP was proven unbounded\n";
      break;
    case (Status::Infeasible):
      std::cerr << "LP was proven infeasible\n";
      break;
    // more cases, see documentation of Status
  }

  return 0;

}
~~~

## More examples

See `examples` directory.