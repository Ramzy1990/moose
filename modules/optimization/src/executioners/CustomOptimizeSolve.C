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

  // if (!_problem.hasUserObject("reporter_user_object"))
  //   mooseError("No DiscreteOptimizationReporter object found.");
  // _reporter = &_problem.getUserObject<DiscreteOptimizationReporter>("reporter_user_object");
  // _objective_function_value = _reporter->getObjectiveInformation();

  // Initial solve
  _inner_solve->solve();

  // Grab objective function
  // if (!_problem.hasUserObject("OptimizationReporter"))
  //   mooseError("No OptimizationReporter object found.");
  // _obj_function = &_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");

  // Initialize solution and matrix
  std::vector<int> iparams;
  std::vector<Real> rparams;
  _reporter->getMeshDomain(iparams, rparams);
  // _obj_function->setInitialCondition(iparams, rparams);

  SimulatedAnnealingAlgorithm * sa_alg =
      dynamic_cast<SimulatedAnnealingAlgorithm *>(_opt_alg.get());
  sa_alg->maxIt() = 20000;
  sa_alg->setInitialSolution({}, iparams);

  // Those are for real!
  // sa_alg->setLowerLimits({-1.2, 2.8});
  // sa_alg->setUpperLimits({-1.1, 2.9});
  // sa_alg->realNeighborSelection() = SimulatedAnnealingAlgorithm::BoxSampling;

  sa_alg->solve();
  //_ndof = _real_parameters.size();
  // bool solveInfo = (taoSolve() == 0);
  std::cout << "ultimate answer " << _opt_alg->intSolution()[0] << " " << _opt_alg->intSolution()[1]
            << std::endl;

  // so the realSolution will give us the _current_real_solution
  auto optimized_vector = _opt_alg->intSolution();
  _reporter->setMeshDomain(optimized_vector);

  return true; // solveInfo;
}

void
CustomOptimizeSolve::objectiveFunctionWrapper(Real & objective,
                                              const std::vector<Real> & rparams,
                                              const std::vector<int> & iparams,
                                              void * ctx)
{

  auto * solver = static_cast<CustomOptimizeSolve *>(ctx);

  // solver->getDiscreteOptimizationReporter().getMeshDomain();

  objective = solver->getDiscreteOptimizationReporter()
                  .getObjectiveInformation(); // call actual objective function here

  std::cout << iparams[0] << " " << iparams[1] << " " << objective << std::endl;
}
