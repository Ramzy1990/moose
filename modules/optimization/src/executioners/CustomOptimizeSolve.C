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

  // DiscreteConstraintsLibrary user object
  // params.addParam<UserObjectName>("constraints_user_object",
  //                                 "The constraints UserObject you want to transfer values "
  //                                 "from.  Note: This might be a UserObject from "
  //                                 "your MultiApp's input file!");

  params.addParam<unsigned int>(
      "number_of_runs",
      "The number of runs allowed for the optimization algorithm (e.g., 10). Every run generates a "
      "new seed number and starts with a new initial guess for the configuration. This initial "
      "guess is the optimal configuration attained from the previous run.");

  params.addParam<unsigned int>("number_of_iterations",
                                "The number of iterations per one run inside the optimization "
                                "algorithm (e.g., 25). This is the maximum allowed per one run.");

  params.addParam<Real>("maximum_temperature",
                        "The maximum temeprature used in the simualted annealing process. "
                        "Higher values allow to check the solution space. ");

  params.addParam<Real>("minimum_temperature",
                        "The minimum temperature used in the simualted annealing process. The "
                        "simulated annealing loop will stop after the number of iterations is "
                        "reached *and* the temperature is below this value. ");
  return params;
}

CustomOptimizeSolve::CustomOptimizeSolve(Executioner & ex)
  : SolveObject(ex),
    _my_comm(MPI_COMM_SELF),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _opt_alg_type(getParam<MooseEnum>("custom_optimizer_type"))
// _constraints(
//     isParamValid("constraints_user_object")
//         ? &_problem.getUserObject<DiscreteConstraintsLibrary>("constraints_user_object")
//         : nullptr)

// _constraints(getUserObject<DiscreteConstraintsLibrary>("constraints_user_object"))
{

  // set up the optimization algorithm
  _opt_alg = std::make_unique<SimulatedAnnealingAlgorithm>();
  _opt_alg->setObjectiveRoutine(objectiveFunctionWrapper, this);

  if (libMesh::n_threads() > 1)
    mooseError("CustomOptimizeSolve does not currently support threaded execution");

  // Reading the number of runs from the input file
  if (isParamValid("number_of_runs"))
    _num_of_runs = getParam<unsigned int>("number_of_runs");
  else
    _num_of_runs = 300;

  // Reading the number of iterations from the input file
  if (isParamValid("number_of_iterations"))
    _num_iterations = getParam<unsigned int>("number_of_iterations");
  else
    _num_iterations = 25;

  // Reading the maximum temperature from the input file
  if (isParamValid("maximum_temperature"))
    _max_temp = getParam<Real>("maximum_temperature");
  else
    _max_temp = 100;

  // Reading the minimum temperature from the input file
  if (isParamValid("minimum_temperature"))
    _min_temp = getParam<Real>("minimum_temperature");
  else
    _min_temp = 0.001;
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
  std::vector<int> exec_params;
  std::map<int, std::vector<int>> elem_neighbors;
  _reporter->getMeshDomain(iparams, rparams, exec_params, elem_neighbors);
  // _obj_function->setInitialCondition(iparams, rparams);

  SimulatedAnnealingAlgorithm * sa_alg =
      dynamic_cast<SimulatedAnnealingAlgorithm *>(_opt_alg.get());

  // sa_alg->setConstraints(_constraints); // Setting _constraints of sa_alg
  sa_alg->maxRun() = _num_of_runs;
  sa_alg->maxIt() = _num_iterations;
  sa_alg->maxTemp() = _max_temp;
  sa_alg->minTemp() = _min_temp;

  sa_alg->setInitialSolution({}, iparams, exec_params, elem_neighbors);

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
  auto run_iteration = solver->_opt_alg->counterRun();

  // get the optimizer name
  std::string optimizer_name = solver->_opt_alg_type.getRawNames();

  // print the table that uses the previous information
  solver->print_table(optimizer_name,
                      iteration,
                      run_iteration,
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
                                 const int run_iteration,
                                 const PostprocessorName objective_name,
                                 const Real objective_value,
                                 bool solution_accepted,
                                 bool tabu_list_used,
                                 bool cache_used)
{
  // int w = 20; // width of each column
  // Calculate the width of each column dynamically
  int w = std::max(
      std::max(custom_optimizer_type.length(), objective_name.length()),
      std::max(std::to_string(iteration).length(), std::to_string(objective_value).length()));
  w += 1; // add some buffer

  // add one more '-' section to cell_boundary_line for the "Run" column
  std::string cell_boundary_line = "+" + std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+" +
                                   std::string(w, '-') + "+" + std::string(w, '-') + "+";

  std::cout << cell_boundary_line << "\n";
  // add "Run" to the column headers, before "Iteration"
  std::cout << "|" << std::left << std::setw(w) << " Optimizer Type"
            << "|" << std::setw(w) << " Objective Function"
            << "|" << std::setw(w) << " Run"
            << "|" << std::setw(w) << " Iteration"
            << "|" << std::setw(w) << " Objective Value"
            << "|" << std::setw(w) << " Solution Accepted?"
            << "|" << std::setw(w) << " Tabu List Used?"
            << "|" << std::setw(w) << " Cache Used?"
            << "|"
            << "\n";
  std::cout << cell_boundary_line << "\n";

  std::cout << "|" << std::left << std::setw(w) << " " + custom_optimizer_type << "|"
            << std::setw(w) << " " + objective_name << "|" << std::setw(w)
            << " " + std::to_string(run_iteration) << "|" << std::setw(w)
            << " " + std::to_string(iteration) << "|" << std::setw(w)
            << " " + std::to_string(objective_value) << "|" << std::setw(w)
            << std::string(" ") + (solution_accepted ? "Yes" : "No") << "|" << std::setw(w)
            << std::string(" ") + (tabu_list_used ? "Yes" : "No") << "|" << std::setw(w)
            << std::string(" ") + (cache_used ? "Yes" : "No") << "|"
            << "\n";

  std::cout << cell_boundary_line << "\n";
}
