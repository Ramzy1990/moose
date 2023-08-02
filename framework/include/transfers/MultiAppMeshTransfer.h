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
#include "MultiAppTransfer.h"

/**
 * A transfer to transfer the mesh from one subApp to its daughter subApp.
 */
class MultiAppMeshTransfer : public MultiAppTransfer
{
public:
  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  MultiAppMeshTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

  //************************
  // Variables Declarations
  //************************
  // Not good as this will use memory and the variable will not be destroyed. Better to make it a
  // method scope variable.
  // /// @brief a variable to store mesh map of the main app.
  // std::map<dof_id_type, subdomain_id_type> _mesh_map;

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

protected:
  //************************
  // Functions Declarations
  //************************
  //
  //

  //************************
  // Variables Declarations
  //************************
  /// @brief If set, indicates a particular subapp to transfer the mesh data to.
  // const unsigned int & _subapp_index;

  /// @brief iterators to count the number of times the transfer has been called. "it" for iteration.
  dof_id_type _it_transfer_to;
  dof_id_type _it_transfer_from;
  dof_id_type _it_transfer;

private:
};
