/*

This example shows the setup and solution of the following linear program:

max x + y + 2z
x + 2y + 3z <= 4
x + y >= 1
x, y, z binary,

using the LinearProgramSolver interface. Switch out the solver
backend to see the different solvers in action; the output
and usage will be the same for each solver.

 */

#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

using namespace lpint;

// Wrapper class, showing the use of the common
// LinearProgramSolver interface
class SolverWrapper {
 public:
  SolverWrapper(std::shared_ptr<LinearProgramSolver> solver)
      : solver_(solver) {}

  std::shared_ptr<LinearProgramSolver> solver() { return solver_; }

 private:
  std::shared_ptr<LinearProgramSolver> solver_;
};

template <typename T>
void print_vector(const std::vector<T>&);

int main() {
  try {
    // Create a linear program object; this will hold all
    // data defining a linear program.
    auto lp = std::make_shared<LinearProgram>(OptimizationType::Maximize,
                                              SparseMatrixType::RowWise);

    // The constraint matrix is set up in CSR format.
    lp->add_rows(
        {Row<double>({1, 2, 3}, {0, 1, 2}), Row<double>({1, 1}, {0, 1})});

    // Add constraints to the LP; constraints consist of a pair
    // (Ordering, value), representing the RHS of the constraint equations
    // including the comparison operator.
    lp->add_constraints({Constraint<double>{Ordering::LEQ, 4.0},
                         Constraint<double>{Ordering::GEQ, 1.0}});

    // Set the objective vector. The objective consists of the
    // coefficients of the elements of x in the expression
    // c^T x, as well as the variable types of the elements
    // of x. These can generally be real, integer, binary, semi-real or
    // semi-integer, depending on what solver one uses (for instance,
    // SoPlex only supports real variables).
    lp->set_objective(Objective<double>{
        {1.0, 1.0, 2.0}, {VarType::Real, VarType::Real, VarType::Real}});

    // Create Gurobi solver object.
    SolverWrapper wrapper(std::make_shared<GurobiSolver>(lp));

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
    // Flush LP data to internal solver.
    // This process keeps the internal LP object intact,
    // so a copy of the coefficient matrix is kept.
    // This may become prohibitively expensive if the
    // coefficient matrix is very large. In such cases,
    // one can use the methods FlushRawData<T>::add_rows()
    // or FlushRawData<T>::add_columns() to directly
    // flush data to the internal solver backend.
    wrapper.solver()->update_program();

    // Solve the primal LP problem:
    Status status = wrapper.solver()->solve_primal();

    if (status != Status::Optimal) {
      std::cout << "Optimal solution NOT found" << std::endl;
    }

    // Retrieve the solution from the solver object.
    Solution<double> solution = wrapper.solver()->get_solution();

    // Check solution content:
    std::cout << "Objective value: " << solution.objective_value << std::endl;
    std::cout << "Solution: ";
    print_vector(solution.primal);

  } catch (const LpException& e) {
    std::cout << e.what() << std::endl;
  }
}

template <typename T>
void print_vector(const std::vector<T>& v) {
  if (v.size() == 0) {
    std::cout << "[]";
    return;
  }
  std::cout << "[";
  for (const auto& el : v) {
    std::cout << el << ", ";
  }
  std::cout << "\b\b]";
}
