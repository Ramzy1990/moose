//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//**********************
// Include Header Files
//**********************
#include "EnclaveCountDiscreteConstraint.h"

//*************************
// Helper Functions if Any
//*************************
// No helper methods to use

//*******************************
// Adding validParams method
//*******************************
InputParameters
EnclaveCountDiscreteConstraint::validParams()
{
  // InputParameters params = emptyInputParameters();
  InputParameters params = ConstraintsLibraryBase::validParams();

  params.addParam<bool>(
      "check_enclaves",
      "This cosntraint checks the enclaves of each material (each subdomain ID). ");

  return params;
}

//*******************
// Class Constructor
//*******************
EnclaveCountDiscreteConstraint::EnclaveCountDiscreteConstraint(const InputParameters & params)
  : ConstraintsLibraryBase(params)
{

  // Debug variable, defaults to off
  _check_enclaves = isParamValid("check_enclaves") ? getParam<bool>("check_enclaves") : 0;
}

//***********************
// Functions Definitions
//***********************
