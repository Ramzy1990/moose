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

/**
 * A class that transfers the mesh from and to the MultiApp system. It is executed on
 * Initial once, and then beginning at each time step.
 * This class uses the DiscreteOptimizationReporter as the user object.
 */
class DiscreteOptimizationTransfer : public MultiAppTransfer
{
public:
  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  DiscreteOptimizationTransfer(const InputParameters & parameters);

  // /**
  //  * Function to help in getting the domain's post processed results names in VectorPostprocessor
  //  * type. Taken from the "SamplerPostprocessorTransfer" class.
  //  * @param[in] prefix: Use the supplied string as the prefix for vector postprocessor name
  //  * @param[in] pp_names: The post prcessors names as supplied from the subapps.
  //  * to print.
  //  */
  // std::vector<VectorPostprocessorName>
  // getVectorNamesHelper(const std::string & prefix, const std::vector<PostprocessorName> &
  // pp_names);

  //
  //
  //
  //
  //
  //
  //
  //************************
  // Variables Declarations
  //************************
  // Nothin to see here

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

protected:
  //************************
  // Functions Declarations
  //************************

  virtual void execute() override;
  virtual void initialSetup() override;

  /**
   * Function to handle the subsequent TO_MULTIAPP use.
   * @param[in] to_mesh: the current mesh to use
   * @param[in] iteration: the current iteration
   */
  void handleSubsequentToInvocations(MooseMesh & to_mesh,
                                     std::vector<MooseMesh *> & to_meshes,
                                     dof_id_type & iteration);

  /**
   * Function to handle the first FROM_MULTIAPP use.
   * @param[in] from_mesh: the current mesh to use
   * @param[in] iteration: the current iteration
   */
  void handleFirstFromInvocation(MooseMesh & from_mesh, dof_id_type & iteration);

  /**
   * Function to handle subsequent FROM_MULTIAPP use.
   * @param[in] from_mesh: the current mesh to use
   * @param[in] iteration: the current iteration
   */
  void handleSubsequentFromInvocations(dof_id_type & iteration);

  /**
   * Function to assign the mesh and update it depending on the optimized map.
   * @param[in] pairs_to_optimize: The optimized pairs
   * @param[in] mesh: the current mesh to update
   */
  void assignMesh(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize,
                  std::vector<MooseMesh *> & meshes);

  /**
   * Function to assign the objective function value from the objective function name, and to set it
   * inside the discrete reporter.
   * @param[in] objective_name: The objective function post processor name as read from the input.
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of the discrete
   * transfer class.
   */
  void objectiveFunctionPP(const PostprocessorName & objective_name, const dof_id_type & iteration);

  /**
   * Function to compare the constraints values and those of the specific pp assocaited with them/
   * @param[in] comparison_results: The comparison results between pp values and constraints values
   * @param[in] postprocessors_values: map of the pp values as assigned in its parent method
   * "buildAndComparePP".
   * @param[in] constraints_values: map of the constraints values as assigned in its parent method
   * "buildAndComparePP".
   * @param[in] constraints_names: The constraints names as read from the input and as
   * found in the subapp. They are basically specific post processors assigned by the user.
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of the discrete
   * transfer class.
   */
  // void printComparisonTable(
  //     const std::vector<bool> & comparison_results,
  //     const std::map<PostprocessorName, PostprocessorValue> & postprocessors_values,
  //     const std::map<PostprocessorName, PostprocessorValue> & constraints_values,
  //     const std::vector<PostprocessorName> & constraints_names,
  //     const dof_id_type & iteration);

  /**
   * Function to print the mesh elements (ids and subdomain ids)
   * @param[in] mesh: The mesh to print its elements
   */
  void printMeshElements(MooseMesh & mesh);

  /**
   * Function to process and verify the from mesh
   * @param[in] mesh: The mesh to process and verify
   */
  void processAndVerifyMesh(MooseMesh & mesh);

  // /**
  //  * Function to print a map of the constraints size or shape.
  //  * @param[in] map: map of pp to print
  //  */
  // void printMap(const std::map<PostprocessorName, PostprocessorValue> & map);

  //
  //
  //
  //
  //
  //
  //
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
  std::vector<subdomain_id_type> _allowed_parameter_values;

  /// @brief excluded subdomains ids in our mesh.
  std::vector<subdomain_id_type> _excluded_parameter_values;

  /// @brief variables describing our mesh.
  /// Mapping between elements and subdomains.
  std::map<dof_id_type, subdomain_id_type> _initial_pairs_to_optimize;
  std::map<dof_id_type, subdomain_id_type> _pairs_to_optimize;

  /// @brief variables that hold the name and value of our objective function.
  PostprocessorName _objective_name;

  /// @brief variable for printing debugging messages
  bool _debug_on;

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

private:
  //************************
  // Functions Declarations
  //************************
  // Nothin to see here

  //
  //
  //
  //
  //
  //
  //
  //************************
  // Variables Declarations
  //************************
};
