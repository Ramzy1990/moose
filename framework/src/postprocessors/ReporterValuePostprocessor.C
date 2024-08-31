//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterValuePostprocessor.h"

registerMooseObject("MooseApp", ReporterValuePostprocessor);

InputParameters
ReporterValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<ReporterName>("reporter",
                                        "The reporter which supplies the postprocessor value.");

  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the reporter");

  params.declareControllable("scale_factor");
  params.addClassDescription("Returns the value of a supplied reporter (scalable)");
  return params;
}

ReporterValuePostprocessor::ReporterValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    // ReporterInterface(this),
    _reporter_value(getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("reporter"))),
    _scale_factor(getParam<Real>("scale_factor"))
{
}

void
ReporterValuePostprocessor::initialize()
{
}

void
ReporterValuePostprocessor::execute()
{
}

Real
ReporterValuePostprocessor::getValue() const
{
  return _scale_factor * _reporter_value[0];
}
