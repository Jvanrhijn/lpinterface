/*

This example shows the setup and solution of the following linear program:

max x + y + 2z
x + 2y + 3z <= 4
x + y >= 1
x, y, z binary

 */

#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

using namespace lpint;

template <typename T>
void print_vector(const std::vector<T>&);

int main() {

  try {

      // Create a linear program object; this will hold all
      // data defining a linear program.
      LinearProgram lp(
          OptimizationType::Maximize,
          SparseMatrixType::RowWise
      );

      // The constraint matrix is set up in CSR format.
      lp.add_rows({
          Row<double>({1, 2, 3}, {0, 1, 2}),
          Row<double>({1, 1}, {0, 1})
      });
    
      // Add constraints to the LP; constraints consist of a pair
      // (Ordering, value), representing the RHS of the constraint equations
      // including the comparison operator.
      lp.add_constraints(
          {
              Constraint<double>{Ordering::LEQ, 4.0},
              Constraint<double>{Ordering::GEQ, 1.0}
          }
      );

      lp.set_objective(
          Objective<double>{
              {1.0, 1.0, 2.0},
              {VarType::Binary, VarType::Binary, VarType::Binary}
          }
      );


      // Create Gurobi solver object. 
      GurobiSolver grb(std::make_shared<LinearProgram>(lp));

      // Flush LP data to internal solver.
      grb.update_program();

      // Solve the primal LP problem:
      Status status = grb.solve_primal();
    
      if (status != Status::Optimal) {
          std::cout << "Optimal solution NOT found" << std::endl;
      }
    
      // Retrieve the solution from the solver object.
      Solution<double> solution = grb.get_solution();

      // Check solution content:
      std::cout << "Objective value: " << solution.objective_value << std::endl;
      std::cout << "Solution: ";
      print_vector(solution.values);

  } catch (const LpException& e) {
      std::cout << e.what() << std::endl;
  }
}

template <typename T>
void print_vector(const std::vector<T>& v) {
    std::cout << "[";
    for (const auto& el: v) {
        std::cout << el << ", ";
    }
    std::cout << "\b\b]";
}