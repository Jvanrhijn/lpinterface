/*
 * This example solve the N-queens problem using Gurobi.
 *
 * The N-queens problem asks how to place N queens on an N x N chess board
 * in such a way that no queen can strike any other. The problem can be solved
 * using integer programming, using the objective
 *
 * max sum(x),
 *
 * where x is the chess board matrix.
 *
 * The constraints that each queen must be safe can be formulated as constraints
 * on the chess board matrix. The four constraints are:
 *
 * - The sum of each row must be less than or equal to 1 (i.e. at most one queen
 * per row).
 * - The sum of each column must be less than or equal to 1.
 * - The sum of each diagonal must be less than or equal to 1.
 * - The sum of each anti-diagonal must be less than or equal to 1.
 */
#include <cmath>
#include <iostream>
#include <numeric>

#include "lpinterface.hpp"
#include "lpinterface/gurobi/lpinterface_gurobi.hpp"

#include "common.hpp"

using namespace lpint;

using Index = Row<double>::Index;

int main(int argc, char *argv[]) {
  // get board width
  std::size_t BOARD_SIZE;
  if (argc < 2) {
    BOARD_SIZE = 8;
  } else if (argv[1] == std::string("-h")) {
    std::cout << "Usage:\n";
    std::cout << "  nqueens [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  n\t\t\t\tWidth of chess board, default 8\n";
    exit(0);
  } else {
    const int size = std::atoi(argv[1]);
    if (size <= 0) {
      std::cerr << "Please provide a positive board width\n";
      exit(0);
    }
    BOARD_SIZE = std::atoi(argv[1]);
  }

  // setup N-queens problem
  auto lp = std::make_shared<LinearProgram>(OptimizationType::Maximize,
                                            SparseMatrixType::RowWise);

  // Objective: maximize number of queens on the board
  // variable types will be binary, so the objective is
  //
  // max sum x
  //
  // where x = vec(X), with X(i, j) binary encoding
  // whether position (i, j) is occupied
  lp->set_objective(Objective<double>{
      std::vector<double>(BOARD_SIZE * BOARD_SIZE, 1.0),
      std::vector<VarType>(BOARD_SIZE * BOARD_SIZE, VarType::Binary)});

  // constraints:
  std::vector<Row<double>> rows;
  std::vector<Constraint<double>> constraints;

  // - max one queen in each row: sum_j X_{ij} <= 1, forall i
  for (std::size_t i = 0; i < BOARD_SIZE; i++) {
    std::vector<Index> nonzeros(BOARD_SIZE);
    std::iota(nonzeros.begin(), nonzeros.end(), i * BOARD_SIZE);
    rows.emplace_back(std::vector<double>(BOARD_SIZE, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  // - max one queen in each column: sum_i X_{ij} <= 1, forall j
  for (std::size_t j = 0; j < BOARD_SIZE; j++) {
    std::vector<Index> nonzeros(BOARD_SIZE);
    for (std::size_t jj = 0; jj < BOARD_SIZE; jj++) {
      nonzeros[jj] = vectorize_indices(jj, j, BOARD_SIZE);
    }
    rows.emplace_back(std::vector<double>(BOARD_SIZE, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  //// - max one queen per sub-diagonal: sum_i X_{i + k, i} <= 1 for k < N
  for (std::size_t k = 0; k < BOARD_SIZE; k++) {
    std::vector<Index> nonzeros(BOARD_SIZE - k);
    for (std::size_t i = 0; i < BOARD_SIZE - k; i++) {
      nonzeros[i] = vectorize_indices(i + k, i, BOARD_SIZE);
    }
    rows.emplace_back(
        std::vector<double>(BOARD_SIZE - k, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  // - max one queen per super-diagonal: sum_i X{i, i + k} <= 1 for k < N
  // skip the actual diagonal for this set of constraints
  for (std::size_t k = 1; k < BOARD_SIZE; k++) {
    std::vector<Index> nonzeros(BOARD_SIZE - k);
    for (std::size_t i = 0; i < BOARD_SIZE - k; i++) {
      nonzeros[i] = vectorize_indices(i, i + k, BOARD_SIZE);
    }
    rows.emplace_back(
        std::vector<double>(BOARD_SIZE - k, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  // - max one queen per anti-sub-diagonal: sum_i X{N - i, i + k + 1} <= 1, k <=
  // N
  for (std::size_t k = 0; k < BOARD_SIZE; k++) {
    std::vector<Index> nonzeros(BOARD_SIZE - k);
    for (std::size_t i = 0; i < BOARD_SIZE - k; i++) {
      nonzeros[i] = vectorize_indices(BOARD_SIZE - i - 1, i + k, BOARD_SIZE);
    }
    rows.emplace_back(
        std::vector<double>(BOARD_SIZE - k, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  // - max one queen per anti-super-diagonal:, sum_i X{N - i - k, i + 1} <= 1, k
  // <= N
  for (std::size_t k = 0; k < BOARD_SIZE; k++) {
    std::vector<Index> nonzeros(BOARD_SIZE - k);
    for (std::size_t i = 0; i < BOARD_SIZE - k; i++) {
      nonzeros[i] = vectorize_indices(BOARD_SIZE - i - 1 - k, i, BOARD_SIZE);
    }
    rows.emplace_back(
        std::vector<double>(BOARD_SIZE - k, 1.0), nonzeros);
    constraints.push_back(Constraint<double>{Ordering::LEQ, 1.0});
  }

  // add rows and constraints to LP
  lp->add_rows(std::move(rows));
  lp->add_constraints(std::move(constraints));

  // create solver
  GurobiSolver gurobi(lp);

  // flush data to backend
  gurobi.update_program();

  // solve program
  auto status = gurobi.solve_primal();
  if (status != Status::Optimal) {
    std::cerr << "Failed to solve N-queens problem\n";
  }
  std::cout << "Solution:\n";

  // display the solution
  print_matrix(vector_to_matrix(gurobi.get_solution().primal));
}
