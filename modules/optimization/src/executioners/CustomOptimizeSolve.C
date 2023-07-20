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
                  .getObjectiveResult(); // call actual objective function here

  // We get the objective name if we need to use it
  PostprocessorName objective_name;
  objective_name = solver->getDiscreteOptimizationReporter()
                       .getObjectiveName(); // call actual objective function here

  // current iteration
  auto iteration = solver->_opt_alg->counterIteration();

  // current objective value
  auto objective_value = objective;

  // if solution is accepted, or tabu list is used, or cache is used
  auto solution_accepted = solver->_opt_alg->tabu();
  auto tabu_list_used = solver->_opt_alg->cache();
  auto cache_used = solver->_opt_alg->solution();

  // get the optimizer name
  std::string optimizer_name = solver->_opt_alg_type.getRawNames();

  // print the table that uses the previous information
  solver->print_table(optimizer_name,
                      iteration,
                      objective_name,
                      objective_value,
                      solution_accepted,
                      tabu_list_used,
                      cache_used);
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
CustomOptimizeSolve::print_table(std::string custom_optimizer_type,
                                 const unsigned int iteration,
                                 const PostprocessorName objective_name,
                                 const Real objective_value,
                                 bool solution_accepted,
                                 bool tabu_list_used,
                                 bool cache_used)
{
  // // int w = 20; // width of each column
  // // Calculate the width of each column dynamically
  // int w = std::max(
  //     std::max(std::to_string(custom_optimizer_type).length(), objective_name.length()),
  //     std::max(std::to_string(iteration).length(), std::to_string(objective_value).length()));
  // w += 5; // add some buffer

  // std::string cell_boundary_line = "+" + std::string(w, '-') + "+" + std::string(w, '-') + "+" +
  //                                  std::string(w, '-') + "+" + std::string(w, '-') + "+" +
  //                                  std::string(w, '-') + "+" + std::string(w, '-') + "+" +
  //                                  std::string(w, '-') + "+";

  // std::cout << cell_boundary_line << "\n";
  // std::cout << "|" << std::left << std::setw(w - 1) << " Optimizer Type"
  //           << "|" << std::setw(w - 1) << " Objective Function"
  //           << "|" << std::setw(w - 1) << " Iteration"
  //           << "|" << std::setw(w - 1) << " Objective Value"
  //           << "|" << std::setw(w - 1) << " Solution Accepted?"
  //           << "|" << std::setw(w - 1) << " Tabu List Used?"
  //           << "|" << std::setw(w - 1) << " Cache Used?"
  //           << "|"
  //           << "\n";
  // std::cout << cell_boundary_line << "\n";

  // std::cout << "|" << std::left << std::setw(w - 1) << " " + custom_optimizer_type << "|"
  //           << std::setw(w - 1) << " " + objective_name << "|" << std::setw(w - 1)
  //           << " " + std::to_string(iteration) << "|" << std::setw(w - 1)
  //           << " " + std::to_string(objective_value) << "|" << std::setw(w - 1)
  //           << std::string(" ") + (solution_accepted ? "Yes" : "No") << "|" << std::setw(w - 1)
  //           << std::string(" ") + (tabu_list_used ? "Yes" : "No") << "|" << std::setw(w - 1)
  //           << std::string(" ") + (cache_used ? "Yes" : "No") << "|"
  //           << "\n";

  // std::cout << cell_boundary_line << "\n";

  std::vector<std::string> headers = {"Optimizer Type",
                                      "Objective Function",
                                      "Iteration",
                                      "Objective Value",
                                      "Solution Accepted?",
                                      "Tabu List Used?",
                                      "Cache Used?"};

  std::vector<std::string> row = {custom_optimizer_type,
                                  objective_name,
                                  std::to_string(iteration),
                                  std::to_string(objective_value),
                                  solution_accepted ? "Yes" : "No",
                                  tabu_list_used ? "Yes" : "No",
                                  cache_used ? "Yes" : "No"};

  std::vector<int> widths;
  for (int i = 0; i < headers.size(); ++i)
  {
    int header_width = headers[i].length();
    int row_width = row[i].length();
    widths.push_back(std::max(header_width, row_width));
  }

  auto print_line = [&]()
  {
    for (int i = 0; i < headers.size(); ++i)
    {
      std::cout << "+-" << std::setw(widths[i]) << std::setfill('-') << "-" << std::setfill(' ');
    }
    std::cout << "+\n";
  };

  auto print_row = [&](const std::vector<std::string> & data)
  {
    for (int i = 0; i < data.size(); ++i)
    {
      std::cout << "| " << std::left << std::setw(widths[i]) << data[i] << " ";
    }
    std::cout << "|\n";
  };

  print_line();
  print_row(headers);
  print_line();
  print_row(row);
  print_line();
}
