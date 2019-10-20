#include "lpinterface/cplex/lpinterface_cplex.hpp"

// macro magic to make cplex behave well (thanks Alberto Santini)
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// end magic

namespace lpint {

CplexSolver::CplexSolver(OptimizationType opt_type)
    : env_(std::make_shared<IloEnv>()), model_(std::make_shared<IloModel>(*env_))
{
  
}

}