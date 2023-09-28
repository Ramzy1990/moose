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

// #include "DensityDiscreteConstraint.h"

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

  params.addParam<bool>("combinatorial_optimization",
                        "If the type of optimization being carried out is combinatorial where we "
                        "reserve the initial volume of each starting material (and hence "
                        "the number of elements with a specific subdomain id). Otherwise, the "
                        "optimization is discrete in that one material is allowed to have more or "
                        "less amount than its initial state to optimize the domain.");

  params.addParam<unsigned int>("dimension", "dimension of the simulated problem.");

  params.addParam<bool>("quarter_symmetry", "If the problem applied has quarter or full symmetry");

  params.addParam<bool>("check_density", "If the density checking constraint is applied.");

  params.addParam<bool>("check_enclaves", "If the enclaves checking constraint is applied.");

  params.addParam<bool>("check_boundaries", "If the boundaries checking constraint is applied.");

  // params.addParam<bool>("check_volumes", "If the volumes checking constraint is applied.");

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

  params.addParam<bool>("debug_on",
                        "The debug variable to print more information during the execution. If on, "
                        "then we print to a file that contains information about different parts "
                        "of the otpimzier and the optimziation process. ");

  return params;
}

CustomOptimizeSolve::CustomOptimizeSolve(Executioner & ex)
  : SolveObject(ex),
    _my_comm(MPI_COMM_SELF),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _opt_alg_type(getParam<MooseEnum>("custom_optimizer_type"))
// _ddc_uo(isParamValid("density_constraint_user_object")
//             ? &getUserObject<DensityDiscreteConstraint>("density_constraint_user_object")
//             : nullptr)

{
  // set up the optimization algorithm
  _opt_alg = std::make_unique<SimulatedAnnealingAlgorithm>();
  _opt_alg->setObjectiveRoutine(objectiveFunctionWrapper, this);

  if (libMesh::n_threads() > 1)
    mooseError("CustomOptimizeSolve does not currently support threaded execution");

  // Is it compinatorial or not
  if (isParamValid("combinatorial_optimization"))
    _combinatorial_optimization = getParam<bool>("combinatorial_optimization");
  else
    _combinatorial_optimization = 0;

  // Checking the dimension
  if (isParamValid("dimension"))
    _dimension = getParam<unsigned int>("dimension");
  else
    _dimension = _problem.mesh().dimension();

  // Quarter symmetry
  if (isParamValid("quarter_symmetry"))
    _quarter_symmetry = getParam<bool>("quarter_symmetry");
  else
    _quarter_symmetry = 1;

  // Checking the density
  if (isParamValid("check_density"))
    _check_density = getParam<bool>("check_density");
  else
    _check_density = 0;

  // Checking the encalves
  if (isParamValid("check_enclaves"))
    _check_enclaves = getParam<bool>("check_enclaves");
  else
    _check_enclaves = 0;

  // Checking the boundaries
  if (isParamValid("check_boundaries"))
    _check_boundaries = getParam<bool>("check_boundaries");
  else
    _check_boundaries = 0;

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

  // Reading the debug variable from the input file
  if (isParamValid("debug_on"))
    _debug = getParam<bool>("debug_on");
  else
    _debug = 0;
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

  sa_alg->combinatorialOptimization() = _combinatorial_optimization;
  sa_alg->meshDimesnsion() = _dimension;
  sa_alg->quarterSymmetry() = _quarter_symmetry;
  sa_alg->checkDensityConstraint() = _check_density;
  sa_alg->checkEnclavesConstraint() = _check_enclaves;
  sa_alg->checkBoundariesConstraint() = _check_boundaries;

  sa_alg->maxRun() = _num_of_runs;
  sa_alg->maxIt() = _num_iterations;
  sa_alg->maxTemp() = _max_temp;
  sa_alg->minTemp() = _min_temp;
  sa_alg->debug() = _debug;

  // Here we set the initial integer configuration subdomains, as well as the execluded subdomains
  // and the neighbors (i.e., neighbor map) of each subdomain.
  // Note that this initialSolution() method is in the custom optimziation algorithm class which the
  // SA inherits from, and we actually casted the sa_alg object from the opt_alg object which was of
  // type custom optimziation algorithm.
  sa_alg->setInitialSolution({}, iparams, exec_params, elem_neighbors);

  // Okay, so if we do not use the following solve(), the optimziation will not work as expected.
  // This is becuase the following solve() of the SA_alg is what makes the optimization process
  // starts. Inside of this solve(), we have the multi-run loops and the while main annealing loop.
  // Both have their iterations (num_of_runs and num_of_iterations, respectively.)
  // This was verified with the pincell problem, where the initial objective function value was
  // compared with a standalone computation and using the logging added to the sa algorithm.
  sa_alg->solve();

  // Here we can do anything after the sa_alg finishes running. maybe we can get some data from the
  // sa_alg to print out to the screen.

  return true; // solveInfo;
}

void
CustomOptimizeSolve::setDensityDiscreteConstraintForAlgorithm(const DensityDiscreteConstraint * ddc)
{
  if (_opt_alg)
  { // Ensure the unique_ptr is not null
    _opt_alg->setDensityDiscreteConstraint(ddc);
  }
}

void
CustomOptimizeSolve::objectiveFunctionWrapper(Real & objective,
                                              const std::vector<Real> & rparams,
                                              const std::vector<int> & iparams,
                                              void * ctx)
{
  // This Wrapper is called everytime inside the simualted annealing algorithm, or so I think!
  auto * solver = static_cast<CustomOptimizeSolve *>(ctx);
  // I think the previous solver object is the current solver which has the _opt_alg and sa_alg
  // inside of it.
  // The opt_alg is the custom optimziation algorithm class.
  // We should add a

  // solver->getDiscreteOptimizationReporter().getMeshDomain();

  // We get the optimized mesh solution
  // There was a big bug here that did not affect the results but only the solution consistency, and
  // the time needed to debug it!!
  /// auto optimized_vector = solver->_opt_alg->intSolution();
  auto optimized_vector = iparams;

  // We set this optimized mesh solution in the reporter
  solver->getDiscreteOptimizationReporter().setMeshDomain(optimized_vector);

  // We then call the updateTheApp function to execute the problem we have!
  // If we do not call this function, the objective function will not be updated at all.
  // It will stay at its initial value.
  // However, the neighbor configuration (neighbor in the subdomain space) will be generated a new
  // everytime. This is becuase the generation of the neighbor is a different process which happens
  // inside the sa_alg->solve() method. So basically, we need this one to be done.
  solver->updateTheApp();

  // We get the objective value! this is from the reporter and it have been set during the transfers
  // execution after the app has finished execution successfully.
  objective = solver->getDiscreteOptimizationReporter()
                  .getObjectiveResult(); // call actual objective function here

  // Now what comes next is basically happening after the app has solved the problem.
  // The solved problem has generated a new objective function, seen above, and finished execution
  // successfully.
  // Next we just print a table that summarizes some of the optimization process that has happened.
  /// We get the objective name if we need to use it
  PostprocessorName objective_name;
  objective_name = solver->getDiscreteOptimizationReporter()
                       .getObjectiveName(); // call actual objective function here

  // current iteration
  auto iteration = solver->_opt_alg->counterIteration();

  // current objective value
  auto objective_value = objective;

  // if solution is accepted, or tabu list is used, or cache is used, and the current run iteration
  auto solution_accepted = solver->_opt_alg->solution();
  auto tabu_list_used = solver->_opt_alg->tabu();
  auto cache_used = solver->_opt_alg->cache();
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

  // This part needs revision.

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
                                 const unsigned int run_iteration,
                                 const PostprocessorName objective_name,
                                 const Real objective_value,
                                 const bool solution_accepted,
                                 const bool tabu_list_used,
                                 const bool cache_used)
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

  std::cout << cell_boundary_line << std::endl;
  // add "Run" to the column headers, before "Iteration"
  std::cout << "|" << std::left << std::setw(w) << " Optimizer Type"
            << "|" << std::setw(w) << " Objective Function"
            << "|" << std::setw(w) << " Run"
            << "|" << std::setw(w) << " Iteration"
            << "|" << std::setw(w) << " Objective Value"
            << "|" << std::setw(w) << " Solution Accepted?"
            << "|" << std::setw(w) << " Tabu List Used?"
            << "|" << std::setw(w) << " Cache Used?"
            << "|" << std::endl;
  std::cout << cell_boundary_line << std::endl;

  std::cout << "|" << std::left << std::setw(w) << " " + custom_optimizer_type << "|"
            << std::setw(w) << " " + objective_name << "|" << std::setw(w)
            << " " + std::to_string(run_iteration) << "|" << std::setw(w)
            << " " + std::to_string(iteration) << "|" << std::setw(w)
            << " " + std::to_string(objective_value) << "|" << std::setw(w)
            << std::string(" ") + (solution_accepted ? "Yes" : "No") << "|" << std::setw(w)
            << std::string(" ") + (tabu_list_used ? "Yes" : "No") << "|" << std::setw(w)
            << std::string(" ") + (cache_used ? "Yes" : "No") << "|" << std::endl;

  std::cout << cell_boundary_line << std::endl;
}
