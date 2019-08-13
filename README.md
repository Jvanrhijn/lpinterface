# lpinterface

[![Build Status](https://travis-ci.org/Jvanrhijn/lpinterface.svg?branch=master)](https://travis-ci.org/Jvanrhijn/lpinterface)
[![codecov](https://codecov.io/gh/Jvanrhijn/lpinterface/branch/master/graph/badge.svg)](https://codecov.io/gh/Jvanrhijn/lpinterface)

Common interface for different linear programming and (mixed-)integer programming solvers.

This library usese a polymorphic interface in order to allow seamless interchange of
linear programming solvers in application code.

### Example usage

In your CMakeLists.txt: `add_subdirectory(path_to_lpinterface)`.

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

template <typename T>
Wrapper create_solver(std::shared_ptr<LinearProgram> lp) {
  return Wrapper(std::make_unique<T>(lp));
}

int main() {
  // construct a simple linear program to maximize an objective function
  // represent the constraint matrix in CSR format
  auto lp = std::make_shared<LinearProgram>(OptimizationType::Maximize);

  // add constraints; these represent the constraint equations.
  // these constraints are equivalent to the equations
  // x + 3y + 3z      <= 1
  //     4y + 5z + 6w <= 4
  // The Row<T> objects are sparse matrix rows in CSR format.
  std::vector<Constraint<double>> constraints;
  constraints.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), Ordering::LEQ, 1.0);
  constraints.emplace_back(Row<double>({4, 5, 6}, {1, 2, 3}), Ordering::LEQ, 4.0);
  lp->add_constraints(std::move(constraints));


  // Set the objective function; see the documentation for a list of
  // supported variable types per solver backend
  lp->set_objective(
    Objective<double>{
      {1, 2, 3, 4}, {VarType::Real, VarType::Integer, VarType::Binary, VarType::Real}
    }
  );

  // Create the polymorphic solver backend
  // switch out the type parameter to change
  // solver backends 
  auto wrapper = create_solver<GurobiSolver>(lp);

  // set solver parameters, see Param documentation for all
  // possible parameter settings
  wrapper.solver->set_parameter(Param::TimeLimit, 10.0);
  
  // flush data to internal LP solver
  wrapper.solver->update_program();

  // solve the primal LP
  const Status status = wrapper.solver->solve_primal();

  // check the solution status
  switch (status) {
    case (Status::Optimal):
      std::cout << "Optimal objective: " 
                <<  wrapper.solver->get_solution().objective_value 
                << std::endl;
      break;
    case (Status::Unbounded):
      std::cerr << "LP was proven unbounded\n";
      break;
    case (Status::Infeasible):
      std::cerr << "LP was proven infeasible\n";
      break;
    // more cases, see documentation of Status
    default:
      std::cerr << "Blah\n";
      break;
  }

  return 0;

}
~~~

### Documentation

jvanrhijn.github.io/lpinterface

### Examples

See `examples` directory.

### Supported solver backends

* Gurobi
* SoPlex
