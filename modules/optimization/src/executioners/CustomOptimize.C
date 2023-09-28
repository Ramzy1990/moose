//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CustomOptimize.h"
#include "DensityDiscreteConstraint.h"

registerMooseObject("OptimizationApp", CustomOptimize);

InputParameters
CustomOptimize::validParams()
{
  InputParameters params = Steady::validParams();
  params += CustomOptimizeSolve::validParams();
  params.addClassDescription("Executioner for optimization problems.");

  // DensityDiscreteConstraint user object
  params.addParam<UserObjectName>("density_constraint_user_object",
                                  "The constraint UserObject you want to use to check a particular "
                                  "cell density in the configuration.");

  return params;
}

CustomOptimize::CustomOptimize(const InputParameters & parameters)
  : Steady(parameters),
    _ddc_uo(isParamValid("density_constraint_user_object")
                ? &getUserObject<DensityDiscreteConstraint>("density_constraint_user_object")
                : nullptr),
    _optim_solve(*this)
{

  // if (isParamValid("density_constraint_user_object"))
  //   _ddc_uo = &getUserObject<DensityDiscreteConstraint>("density_constraint_user_object");
  // else
  //   _ddc_uo = nullptr;

  // Pass the DensityDiscreteConstraint object to CustomOptimizeSolve
  _optim_solve.setDensityDiscreteConstraintForAlgorithm(_ddc_uo);

  _optim_solve.setInnerSolve(fixedPointSolve());
}

void
CustomOptimize::execute()
{
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  _problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

  _problem.timestepSetup();

  // Here we call the solve() method of the custom optimization solve class.
  // So anything happens inside the solve method there is applied inside the solve() method. So for
  // example, the loops and everything happening there is but just a single command as follows. We
  // can then try to write anything after this to see if it is indeed the case.
  _optim_solve.solve();

  std::cout << "This should prints after the very final run iteration!" << std::endl;
  // Yup, it did print out!!

  /// need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  _problem.execute(EXEC_TIMESTEP_END);
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  _problem.execMultiApps(EXEC_FINAL);
  _problem.finalizeMultiApps();
  _problem.execute(EXEC_FINAL);
  _time = _time_step;
  _problem.outputStep(EXEC_FINAL);
  _time = _system_time;

  postExecute();
}
