//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
// #include "ReporterInterface.h" # General postprocessor has this reporter interface inherited
// already! Source of ambguity will appear if compiled.

// class ReporterInterface;

/**
 * This postprocessor displays a single value which is supplied by a Reporter!
 * It is designed to allow having the reporter value as a postprocessor.
 This is a postprocessor that takes a reporter as input and returns the value of the reporter as the
 postprocessor value. It is designed for the postprocessor have a dedicated transfer to an auxilary
 variable. This can be used such that the auxilary variable will have the value of the reporter
 which then can be used in some physics (e.g., as a parameter state for XS interpolation).
 */
class ReporterValuePostprocessor : public GeneralPostprocessor //, public ReporterInterface
{
public:
  static InputParameters validParams();

  ReporterValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;

protected:
  /// the reporter value that we will get and returned as pp value
  const std::vector<Real> & _reporter_value;

  /// a scale factor to scale the result of _function
  const Real & _scale_factor;
};
