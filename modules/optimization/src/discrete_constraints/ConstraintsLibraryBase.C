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
#include "ConstraintsLibraryBase.h"

//*************************
// Helper Functions if Any
//*************************
// No helper methods to use

//*******************************
// Adding validParams method
//*******************************
InputParameters
ConstraintsLibraryBase::validParams()
{
  // InputParameters params = emptyInputParameters();
  InputParameters params = MooseObject::validParams();

  params.registerBase("Cons");

  return params;
}

//*******************
// Class Constructor
//*******************
ConstraintsLibraryBase::ConstraintsLibraryBase(const InputParameters & params) : MooseObject(params)
{
}

//***********************
// Functions Definitions
//***********************
