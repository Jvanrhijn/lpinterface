#include "lpinterface/gurobi/lpinterface_gurobi.hpp"
#include <iostream>

namespace lpint {

GurobiSolver::GurobiSolver(std::shared_ptr<LinearProgramInterface> lp)
    : linear_program_(lp) {
  // load environment
  GRBloadenv(&gurobi_env_, "");
 // // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  set_parameter(Param::GrbOutputFlag, 0);

 // // set optimization type
 GRBsetintattr(
     gurobi_model_, "modelsense",
     linear_program_->optimization_type() == OptimizationType::Maximize
         ? GRB_MAXIMIZE
         : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(OptimizationType opt_type) {
  GRBloadenv(&gurobi_env_, "");
  // allocate Gurobi model
  GRBnewmodel(gurobi_env_, &gurobi_model_, nullptr, 0, nullptr, nullptr,
              nullptr, nullptr, nullptr);
  set_parameter(Param::GrbOutputFlag, 0);

  // set optimization type
  GRBsetintattr(
      gurobi_model_, "modelsense",
      opt_type == OptimizationType::Maximize ? GRB_MAXIMIZE : GRB_MINIMIZE);
}

GurobiSolver::GurobiSolver(const GurobiSolver& other) noexcept
    : linear_program_(other.linear_program_) {
  gurobi_model_ = GRBcopymodel(other.gurobi_model_);
  gurobi_env_ = GRBgetenv(gurobi_model_);
}

GurobiSolver& GurobiSolver::operator=(GurobiSolver other) noexcept {
  swap(*this, other);
  return *this;
}

GurobiSolver::GurobiSolver(GurobiSolver&& other) noexcept : GurobiSolver() {
  swap(*this, other);
}

GurobiSolver::~GurobiSolver() {
  // Free Gurobi resources
  GRBfreeenv(GRBgetenv(gurobi_model_));
  GRBfreemodel(gurobi_model_);
}

void GurobiSolver::set_parameter(const Param param, const int value) {
  if (auto error = GRBsetintparam(GRBgetenv(gurobi_model_),
                                  translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::set_parameter(const Param param, const double value) {
  if (auto error = GRBsetdblparam(GRBgetenv(gurobi_model_),
                                  translate_parameter(param), value)) {
    throw GurobiException(error);
  }
}

void GurobiSolver::update_program() {
  if (!linear_program_->is_initialized()) {
    throw LinearProgramNotInitializedException();
  }
  // first add variables to Gurobi
  auto objective = linear_program_->objective();
  std::size_t num_vars = objective.values.size();
  auto vt = convert_variable_type(objective.variable_types);
  auto err =
      GRBaddvars(gurobi_model_, num_vars, 0, nullptr, nullptr, nullptr,
                 objective.values.data(), nullptr, nullptr, vt.data(), nullptr);
  if (err != 0) {
    throw GurobiException(err);
  }
  // set constraints
  auto matrix = linear_program_->matrix();
  auto constraints = linear_program_->constraints();
  std::size_t idx = 0;
  if (matrix.type() == SparseMatrixType::RowWise) {
    for (auto& row : matrix) {
      char ord = convert_ordering(constraints[idx].ordering);
      // need to do this since gurobi wants int* for indices
      // for some ungodly reason
      std::vector<int> nonzero_indices(row.nonzero_indices().begin(),
                                       row.nonzero_indices().end());
      auto error =
          GRBaddconstr(gurobi_model_, row.num_nonzero(), nonzero_indices.data(),
                       row.values().data(), ord, constraints[idx].value,
                       ("constr" + std::to_string(idx)).c_str());
      if (error != 0) {
        throw GurobiException(error);
      }
      idx++;
    }
  } else {
    throw MatrixTypeException();
  }
}

Status GurobiSolver::solve_primal() {
  auto error = GRBoptimize(gurobi_model_);
  if (error) {
    throw GurobiException(error);
  }
  error = GRBgetdblattr(gurobi_model_, GRB_DBL_ATTR_OBJVAL,
                        &solution_.objective_value);
  if (error) {
    throw GurobiException(error);
  }
  int num_vars;
  error == GRBgetintattr(gurobi_model_, GRB_INT_ATTR_NUMVARS, &num_vars);
  solution_.primal.resize(static_cast<std::size_t>(num_vars));
  error = GRBgetdblattrarray(gurobi_model_, GRB_DBL_ATTR_X, 0, num_vars,
                             solution_.primal.data());
  if (error) {
    throw GurobiException(error);
  }
  return solution_status();
}

Status GurobiSolver::solve_dual() { throw UnsupportedFeatureException(); }

Status GurobiSolver::solution_status() const {
  int status;
  auto error = GRBgetintattr(gurobi_model_, GRB_INT_ATTR_STATUS, &status);
  if (error) {
    throw GurobiException(error);
  }
  return convert_gurobi_status(status);
}

const LinearProgramInterface& GurobiSolver::linear_program() const {
  if (!linear_program_->is_initialized()) {
    throw LinearProgramNotInitializedException();
  }
  return *linear_program_;
}

LinearProgramInterface& GurobiSolver::linear_program() {
  if (!linear_program_->is_initialized()) {
    throw LinearProgramNotInitializedException();
  }
  return *linear_program_;
}

const Solution<double>& GurobiSolver::get_solution() const { return solution_; }

void GurobiSolver::add_columns(
    __attribute__((unused)) std::vector<double>&& values,
    __attribute__((unused)) std::vector<int>&& start_indices,
    __attribute__((unused)) std::vector<int>&& row_indices,
    __attribute__((unused)) std::vector<Ordering>&& ord,
    __attribute__((unused)) std::vector<double>&& rhs) {
  throw UnsupportedFeatureException();
}

void GurobiSolver::add_rows(std::vector<double>&& values,
                            std::vector<int>&& start_indices,
                            std::vector<int>&& col_indices,
                            std::vector<Ordering>&& ord,
                            std::vector<double>&& rhs) {
  std::vector<char> ord_grb;
  for (const auto& ordering : ord) {
    ord_grb.push_back(convert_ordering(ordering));
  }
  auto error = GRBaddconstrs(
      gurobi_model_, start_indices.size(), values.size(), start_indices.data(),
      col_indices.data(), values.data(), ord_grb.data(), rhs.data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}
void GurobiSolver::add_variables(std::vector<double>&& objective_values,
                                 std::vector<VarType>&& var_types) {
  auto error =
      GRBaddvars(gurobi_model_, objective_values.size(), 0, nullptr, nullptr,
                 nullptr, objective_values.data(), nullptr, nullptr,
                 convert_variable_type(var_types).data(), nullptr);
  if (error) {
    throw GurobiException(error);
  }
}

std::vector<char> GurobiSolver::convert_variable_type(
    const std::vector<VarType>& var_types) {
  auto num_vars = var_types.size();
  std::vector<char> value_type(num_vars);
  for (std::size_t i = 0; i < num_vars; i++) {
    char vtype;
    switch (var_types[i]) {
      case VarType::Binary:
        vtype = GRB_BINARY;
        break;
      case VarType::Integer:
        vtype = GRB_INTEGER;
        break;
      case VarType::Real:
        vtype = GRB_CONTINUOUS;
        break;
      case VarType::SemiReal:
        vtype = GRB_SEMICONT;
        break;
      case VarType::SemiInteger:
        vtype = GRB_SEMIINT;
        break;
      default:
        throw UnsupportedVariableTypeException();
    }
    value_type[i] = vtype;
  }
  return value_type;
}

constexpr char GurobiSolver::convert_ordering(const Ordering ord) {
  switch (ord) {
    case Ordering::LEQ:
      return GRB_LESS_EQUAL;
    case Ordering::GEQ:
      return GRB_GREATER_EQUAL;
    case Ordering::EQ:
      return GRB_EQUAL;
    default:
      throw UnsupportedConstraintException();
  }
}

constexpr Status GurobiSolver::convert_gurobi_status(int status) {
  switch (status) {
    case GRB_LOADED:
      return Status::NoInformation;
    case GRB_OPTIMAL:
      return Status::Optimal;
    case GRB_INFEASIBLE:
      return Status::Infeasible;
    case GRB_INF_OR_UNBD:
      return Status::InfeasibleOrUnbounded;
    case GRB_UNBOUNDED:
      return Status::Unbounded;
    case GRB_CUTOFF:
      return Status::Cutoff;
    case GRB_ITERATION_LIMIT:
      return Status::IterationLimit;
    case GRB_NODE_LIMIT:
      return Status::NodeLimit;
    case GRB_TIME_LIMIT:
      return Status::TimeOut;
    case GRB_SOLUTION_LIMIT:
      return Status::SolutionLimit;
    case GRB_INTERRUPTED:
      return Status::Interrupted;
    case GRB_NUMERIC:
      return Status::NumericFailure;
    case GRB_SUBOPTIMAL:
      return Status::SuboptimalSolution;
    case GRB_INPROGRESS:
      return Status::InProgress;
    case GRB_USER_OBJ_LIMIT:
      return Status::UserObjectiveLimit;
    default:
      throw UnknownStatusException();
  }
}

// TODO: extend
constexpr const char* GurobiSolver::translate_parameter(const Param param) {
  switch (param) {
    case (Param::GrbOutputFlag):
      return "outputflag";
    case (Param::GrbThreads):
      return "threads";
    case (Param::Cutoff):
      return "Cutoff";
    default:
      throw UnsupportedParameterException();
  }
}

}  // namespace lpint