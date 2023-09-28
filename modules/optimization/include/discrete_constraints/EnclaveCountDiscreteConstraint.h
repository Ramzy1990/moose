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

class EnclaveCountDiscreteConstraint : public ConstraintsLibraryBase
{
public:
  //************************
  // Functions Declarations
  //************************

  static InputParameters validParams();

  /**
   * Constructor and destructor.
   */
  EnclaveCountDiscreteConstraint(const InputParameters & parameters);
  // virtual ~DiscreteConstraintsUtils() = default;

  // // Getter function to access _check_enclaves
  bool shouldCheckEnclaves() const { return _check_enclaves; }

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/

protected:
  //************************
  // Functions Declarations
  //************************

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************

  /// @brief variable for checking the number of enclaves for each material
  bool _check_enclaves;

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
};
