//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusOptimizationEigenvalue.h"

registerMooseObject("OptimizationApp", ExodusOptimizationEigenvalue);

InputParameters
ExodusOptimizationEigenvalue::validParams()
{
  // Get the base class parameters
  InputParameters params = Exodus::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  // Return the InputParameters
  return params;
}

ExodusOptimizationEigenvalue::ExodusOptimizationEigenvalue(const InputParameters & parameters)
  : Exodus(parameters), _eigenvalue_exec(dynamic_cast<Eigenvalue *>(_app.getExecutioner()))
{
  if (!_eigenvalue_exec)
    mooseError("ExodusOptimizationEigenvalue output can only be used with the Eigenvalue or "
               "related executioners to output a per-optimization iteration solution.");
}

void
ExodusOptimizationEigenvalue::customizeFileOutput()
{
  if (_exodus_mesh_changed || _sequence)
    _file_num++;

  _exodus_num = _eigenvalue_exec->getIterationNumberOutput() + 1;

  if (_exodus_num == 1)
    _exodus_io_ptr->append(false);
  else
    _exodus_io_ptr->append(true);
}

Real
ExodusOptimizationEigenvalue::getOutputTime()
{
  return _eigenvalue_exec->getIterationNumberOutput();
}
