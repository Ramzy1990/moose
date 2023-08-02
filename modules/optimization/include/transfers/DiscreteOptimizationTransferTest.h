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

// Forward Declarations If Any
class DiscreteOptimizationReporter;

// Oh well, let us see in the meeting today then.
// Speaking of postprocessing, is the user able to assign a postprocessor object from the objects
// available in postprocessors in a basic mathematical function? Let us say something of the
// following form: 2y + 3/x where y and x are some postprocessor available function that the user
// use to get variables from his subapp problem. Like

/**
 * A class that transfers the mesh from and to the MultiApp system. It is executed on
 * Initial once, and then beginning at each time step.
 * This class uses the DiscreteOptimizationReported as the user object.
 */
class DiscreteOptimizationTransferTest : public MultiAppTransfer
{
public:
  //************************
  // Variables Declarations
  //************************
  // Nothin to see here

  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  DiscreteOptimizationTransferTest(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialSetup() override;

  /**
   * Function to assign the mesh
   */
  void assignMesh(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize,
                  MooseMesh & mesh);

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

protected:
  //************************
  // Variables Declarations
  //************************

  /// @brief Object of the reporter class we are using. I think this will allow us to access the different variables and functions in
  /// the reporter by the "." operator.
  // const DiscreteOptimizationReporter * const _reporter;
  // DiscreteOptimizationReporter & _reporter;
  DiscreteOptimizationReporter * _reporter;

  /// @brief mesh from the _to_problems
  // MooseMesh & _to_mesh;

  /// @brief mesh from the _from_problem // Dummy one mesh
  // MooseMesh & _from_mesh;

  /// @brief the class variable that contains the user object name. This is DiscreteOptimizationReporter
  // std::string _user_object_name;

  /// @brief a variable to allow execution at initial once for the "from multiApp" logical branch.
  // bool _skip;

  /// @brief iterators to count the number of times the transfer has been called. "it" for iteration.
  dof_id_type _it_transfer_to;
  dof_id_type _it_transfer_from;
  dof_id_type _it_transfer;

  /// @brief allowed subdomains ids in our mesh.
  std::vector<subdomain_id_type> _transfer_allowed_parameter_values;

  /// @brief excluded subdomains ids in our mesh.
  std::vector<subdomain_id_type> _transfer_excluded_parameter_values;

  /// @brief variables describing our mesh.
  /// Mapping between elements and subdomains.
  std::map<dof_id_type, subdomain_id_type> _transfer_initial_pairs_to_optimize;
  std::map<dof_id_type, subdomain_id_type> _transfer_pairs_to_optimize;

  //************************
  // Functions Declarations
  //************************
  // Nothin to see here

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

private:
  //************************
  // Variables Declarations
  //************************
  // Nothin to see here

  //************************
  // Functions Declarations
  //************************
  // Nothin to see here
};
