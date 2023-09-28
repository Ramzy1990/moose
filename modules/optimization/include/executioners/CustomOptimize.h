//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "Steady.h"
#include "CustomOptimizeSolve.h"

// System includes
#include <string>

// Forward Declarations If Any
class InputParameters;
class DensityDiscreteConstraint;
class FEProblemBase;

class CustomOptimize : public Steady
{
public:
  static InputParameters validParams();

  CustomOptimize(const InputParameters & parameters);

  virtual void execute() override;

  // DensityDiscreteConstraint * getDensityDiscreteConstraint() override
  // {
  //   return &_ddc_uo; // Returns address of the ThirdParty object
  // }

  CustomOptimizeSolve & getOptimizeSolve() { return _optim_solve; }

  // could be not needed!
  const DensityDiscreteConstraint & getDensityConstraint() const { return *_ddc_uo; }

protected:
  CustomOptimizeSolve _optim_solve;

  /// @brief Object of the density constraint class
  const DensityDiscreteConstraint * _ddc_uo;
};
