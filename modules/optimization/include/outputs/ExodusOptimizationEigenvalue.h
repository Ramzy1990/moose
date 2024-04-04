//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Exodus.h"
#include "Eigenvalue.h"

/**
 * Class for output data to the ExodusII format
 */
class ExodusOptimizationEigenvalue : public Exodus
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  ExodusOptimizationEigenvalue(const InputParameters & parameters);

protected:
  /**
   * Get time for exodus output (i.e. iteration number for optimization solves).
   */
  virtual Real getOutputTime() override;

  /**
   * Customize file output with the converged eigenvalue iteration number
   */
  void customizeFileOutput() override;

private:
  /// For eigenvalue executioner
  const Eigenvalue * const _eigenvalue_exec;
};
