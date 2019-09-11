/*

This example shows the setup and solution of the following linear program:

max x + y + 2z
x + 2y + 3z <= 4
x + y >= 1
x, y, z real,

using the LinearProgramSolver interface. Switch out the solver
backend to see the different solvers in action; the output
and usage will be the same for each solver.

 */

#include <iostream>

#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include "lpinterface/soplex/lpinterface_soplex.hpp"

#include "common.hpp"

using namespace lpint;

// Wrapper class, showing the use of the common
// LinearProgramSolver interface
class SolverWrapper {
 public:
  SolverWrapper() = default;

  explicit SolverWrapper(std::shared_ptr<LinearProgramSolver> solver)
      : solver_(solver) {}

  void insert_solver(std::shared_ptr<LinearProgramSolver> solver) {
    solver_ = solver;
  }

  std::shared_ptr<LinearProgramSolver> solver() { return solver_; }

 private:
  std::shared_ptr<LinearProgramSolver> solver_;
};

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Please provide a solver backend as argument. Supported: "
                 "gurobi, soplex."
              << std::endl;
    exit(1);
  }

  try {
    // Create solver object.
    SolverWrapper wrapper;

    if (argv[1] == std::string("gurobi")) {
      wrapper.insert_solver(std::make_shared<GurobiSolver>());
    } else if (argv[1] == std::string("soplex")) {
      wrapper.insert_solver(std::make_shared<SoplexSolver>());
    } else {
      std::cerr << "Unsupported solver backend." << std::endl;
    }

    wrapper.solver()->set_parameter(Param::Verbosity, 0);

    auto& lp = wrapper.solver()->linear_program();

    // Create a linear program object; this will hold all
    // data defining a linear program.
    lp.set_objective_sense(OptimizationType::Maximize);

    // Add 3 variables to the LP.
    lp.add_variables(3);

    // Set the objective vector. The objective consists of the
    // coefficients of the elements of x in the expression
    // c^T x, i.e. the elements of c.
    lp.set_objective(Objective<double>({1.0, 1.0, 2.0}));

    // The constraint matrix is set up in CSR format.
    // Each Constraint consists of a matrix row, an
    // ordering, and a bound.
    std::vector<Constraint<double>> constraints;
    constraints.emplace_back(Row<double>({1, 2, 3}, {0, 1, 2}), -LPINT_INFINITY, 4.0);
    constraints.emplace_back(Row<double>({1, 1}, {0, 1}), 1.0, LPINT_INFINITY);
    lp.add_constraints(constraints);

    /*
        Alternatively, one can directly flush the data to the LP solver
        using the FlushRawData<T> interface. In this case, you should
        specify the optimization type when constructing the LP solver
        object, as this is no longer specified in the LP object.

          SolverWrapper
       wrapper(std::make_shared<GurobiSolver>(OptimizationType::Maximize));

          {
            auto flusher =
       std::dynamic_pointer_cast<FlushRawData<double>>(wrapper.solver());

            flusher->add_variables(
                {1.0, 1.0, 2.0},
                {VarType::Binary, VarType::Binary, VarType::Binary}
            );

            flusher->add_rows(
                {1, 2, 3, 1, 1},
                {0, 3},
                {0, 1, 2, 0, 1},
                {Ordering::LEQ, Ordering::GEQ},
                {4, 1}
            );
          }
    */

    // Solve the primal LP problem:
    Status status = wrapper.solver()->solve();

    if (status != Status::Optimal) {
      std::cerr << "Optimal solution NOT found" << std::endl;
      exit(1);
    }

    // Retrieve the solution from the solver object.
    Solution<double> solution = wrapper.solver()->get_solution();

    // Check solution content:
    std::cout << "Objective value: " << solution.objective_value << std::endl;
    std::cout << "Solution: ";
    print_vector(solution.primal);
    std::cout << "\nDual solution: ";
    print_vector(solution.dual);

  } catch (const LpException& e) {
    std::cout << e.what() << std::endl;
  }
}