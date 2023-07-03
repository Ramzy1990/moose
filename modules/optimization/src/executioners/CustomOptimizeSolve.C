//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CustomOptimizeSolve.h"
#include "OptimizationAppTypes.h"
#include "OptimizationReporterBase.h"
#include "DiscreteOptimizationReporter.h" // declared in the header file
#include "CustomOptimizationAlgorithm.h"
#include "SimulatedAnnealingAlgorithm.h"

InputParameters
CustomOptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  ExecFlagEnum exec_enum = ExecFlagEnum();

  exec_enum.addAvailableFlags(EXEC_NONE,
                              OptimizationAppTypes::EXEC_FORWARD,
                              OptimizationAppTypes::EXEC_ADJOINT,
                              OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);
  exec_enum = {OptimizationAppTypes::EXEC_FORWARD,
               OptimizationAppTypes::EXEC_ADJOINT,
               OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD};
  params.addParam<ExecFlagEnum>(
      "solve_on", exec_enum, "List of flags indicating when inner system solve should occur.");

  MooseEnum custom_optimizer_type("simulated_annealing", "simulated_annealing");
  params.addParam<MooseEnum>(
      "custom_optimizer_type", custom_optimizer_type, "The optimization algorithm.");

  // DiscreteOptimizationReporter user object
  params.addRequiredParam<UserObjectName>("reporter_user_object",
                                          "The Reporter UserObject you want to transfer values "
                                          "from.  Note: This might be a UserObject from "
                                          "your MultiApp's input file!");

  return params;
}

CustomOptimizeSolve::CustomOptimizeSolve(Executioner & ex)
  : SolveObject(ex),
    _my_comm(MPI_COMM_SELF),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _opt_alg_type(getParam<MooseEnum>("custom_optimizer_type"))
{
  // set up the optimization algorithm
  _opt_alg = std::make_unique<SimulatedAnnealingAlgorithm>();
  _opt_alg->setObjectiveRoutine(objectiveFunctionWrapper, this);

  if (libMesh::n_threads() > 1)
    mooseError("CustomOptimizeSolve does not currently support threaded execution");
}

bool
CustomOptimizeSolve::solve()
{
  TIME_SECTION("CustomOptimizeSolve", 1, "Custom optimization solve");

  _reporter = &_problem.getUserObject<DiscreteOptimizationReporter>(
      getParam<UserObjectName>("reporter_user_object"));

  // Initial solve
  _inner_solve->solve();

  // Initialize solution and matrix
  std::vector<int> iparams;
  std::vector<Real> rparams;
  _reporter->getMeshDomain(iparams, rparams);
  // _obj_function->setInitialCondition(iparams, rparams);
  SimulatedAnnealingAlgorithm * sa_alg =
      dynamic_cast<SimulatedAnnealingAlgorithm *>(_opt_alg.get());

  sa_alg->maxIt() = 25;

  sa_alg->setInitialSolution({}, iparams);

  sa_alg->solve();

  // std::cout << "ultimate answer " << sa_alg->intSolution()[0] << " " << sa_alg->intSolution()[1]
  //           << std::endl;

  // // Print the map
  // for (const auto & i : sa_alg->intSolution())
  // {
  //   std::cout << "ultimate mesh subdomain ids:  " << i << " " << std::endl;
  // }

  return true; // solveInfo;
}

void
CustomOptimizeSolve::objectiveFunctionWrapper(Real & objective,
                                              const std::vector<Real> & rparams,
                                              const std::vector<int> & iparams,
                                              void * ctx)
{

  // This Wrapper is called everytime inside the simualted annealing algorithm, or so I think!
  auto * solver = static_cast<CustomOptimizeSolve *>(ctx);

  // solver->getDiscreteOptimizationReporter().getMeshDomain();

  // We get the optimized mesh solution
  auto optimized_vector = solver->_opt_alg->intSolution();

  // We set this optimized mesh solution in the reporter
  solver->getDiscreteOptimizationReporter().setMeshDomain(optimized_vector);

  // We then call the updateTheApp function to execute the problem we have!
  solver->updateTheApp();

  // We get the objective value! this is from the rpeorter and it have been set during the transfers
  // execution
  objective = solver->getDiscreteOptimizationReporter()
                  .getObjectiveInformation(); // call actual objective function here

  // solver->print_table(solver->_opt_alg_type, solver->_opt_alg);
  // std::cout << iparams[0] << " " << iparams[1] << " " << objective << "\n\n\n";
}

void
CustomOptimizeSolve::updateTheApp()
{
  TIME_SECTION("updateTheApp", 2, "Objective forward solve");

  // execute the MultiApp!
  _problem.execute(OptimizationAppTypes::EXEC_FORWARD);

  // _problem.restoreMultiApps(OptimizationAppTypes::EXEC_FORWARD);

  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_FORWARD))
    mooseError("Forward solve multiapp failed!");
  if (_solve_on.contains(OptimizationAppTypes::EXEC_FORWARD))
    _inner_solve->solve();
}

void
CustomOptimizeSolve::print_table(MooseEnum custom_optimizer_type,
                                 //  std::string objective_function,
                                 int iteration,
                                 Real objective_value,
                                 std::string optimized_mesh,
                                 std::string solution_accepted,
                                 std::string tabu_list_used,
                                 std::string cache_used,
                                 bool is_first = false)
{
  int w = 20; // width of each column
  std::string cell_boundary_line = "+" + std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+";

  if (is_first)
  {
    std::cout << cell_boundary_line << "\n";
    std::cout << "|" << std::left << std::setw(w - 1)
              << " Optimizer Type"
              // << "|" << std::setw(w - 1) << " Objective Function"
              << "|" << std::setw(w - 1) << " Iteration"
              << "|" << std::setw(w - 1) << " Objective Value"
              << "|" << std::setw(w - 1) << " Optimized Mesh"
              << "|" << std::setw(w - 1) << " Solution Accepted?"
              << "|" << std::setw(w - 1) << " Tabu List Used?"
              << "|" << std::setw(w - 1) << " Cache Used"
              << "|"
              << "\n";
    std::cout << cell_boundary_line << "\n";
  }

  std::cout << "|" << std::left << std::setw(w - 1) << " " + custom_optimizer_type
            << "|"
            // << std::setw(w - 1) << " " + objective_function << "|" << std::setw(w - 1)
            << " " + std::to_string(iteration) << "|" << std::setw(w - 1)
            << " " + std::to_string(objective_value) << "|" << std::setw(w - 1)
            << " " + optimized_mesh << "|" << std::setw(w - 1) << " " + solution_accepted << "|"
            << std::setw(w - 1) << " " + tabu_list_used << "|" << std::setw(w - 1)
            << " " + cache_used << "|"
            << "\n";

  std::cout << cell_boundary_line << "\n";
}
