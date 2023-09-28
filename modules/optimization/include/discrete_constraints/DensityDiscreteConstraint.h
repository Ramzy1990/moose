//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

//**********************
// Include Header Files
//**********************
#include "ConstraintsLibraryBase.h"

//*****************************
// Forward Declarations If Any
//*****************************
// No forward declarations

class DensityDiscreteConstraint : public ConstraintsLibraryBase
{
public:
  //************************
  // Functions Declarations
  //************************

  static InputParameters validParams();

  /**
   * Constructor and destructor.
   */
  DensityDiscreteConstraint(const InputParameters & parameters);
  // virtual ~DiscreteConstraintsUtils() = default;

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};

  bool checkConfiguration() override;

  // Getter function to access _check_density. One can also put the _check_density variable under
  // public and access it directly.
  bool shouldCheckDensity() const { return _check_density; }

protected:
  //************************
  // Functions Declarations
  //************************

  Real
  checkBoundingBoxDensity(const std::vector<int> & int_vec, int cellType, unsigned int dimension);

  //************************
  // Variables Declarations
  //************************

  /// @brief variable for checking the density of the bounding box around cells of each type
  bool _check_density;
};
