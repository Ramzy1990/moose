//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "GeneralReporter.h"

// Forward Declarations If Any
// class FEProblemBase;
// class MooseMesh;

/**
 * Contains reporter for communicating between CustomOptimizeSolve and subapps.
 */
class DiscreteOptimizationReporter : public GeneralReporter
//  public ElementUserObject

{
public:
  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  // Constructor
  DiscreteOptimizationReporter(const InputParameters & parameters);

  void execute() override {}

  /**
   * Function to check if the materials used are the ones allowed
   * @param[in] domain_mesh: the domain's mesh optimized by the optimizer
   */
  void isMaterialAllowed(const MooseMesh & domain_mesh);

  /**
   * Function to initialize the subdomain id vectors including pairs_to_optimize and the neighbors
   * @param[in] domain_mesh: the domain's mesh optimized by the optimizer
   */
  void setInitialCondition(const MooseMesh & domain_mesh);

  /**
   * Function to set _pairs_to_optimize values from the optimizer.
   * @param[in] optimized_vector: the optimized configuration as generated by the optimizer
   */
  void setMeshDomain(const std::vector<int> & optimized_vector);

  /**
   * Function to get the mesh domain from the reporter to the optimizer. (Basically used for the
   * initial condition only)
   * @param[inout] iparams: the integer parameters vector that will be optimized.
   * @param[inout] rparams: the real parameters vector that will be optimized.
   * @param[inout] exec_params: the integer parameters vector that will be execluded.
   * @param[inout] elem_neighbors: the integer parameters map that holds the neighbors information.
   */
  void getMeshDomain(std::vector<int> & iparams,
                     std::vector<Real> & rparams,
                     std::vector<int> & exec_params,
                     std::map<int, std::vector<int>> & elem_neighbors);

  /**
   * Function to set the objective function value in the reporter.
   * @param[in] objective_name: name of the objective function 🎯 for the
   * optimizer to get
   * @param[in] objective_result: the value of the objective function 🎯 for the
   * optimizer  to get
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of
   * the discrete Transfer class.
   */
  void setObjectiveInformation(const PostprocessorName & objective_name,
                               const Real & objective_result,
                               const dof_id_type & iteration);

  /**
   * Function to get the constraints comparison results from the reporter in the
   * optimizer.
   */
  Real getObjectiveResult() const;

  /**
   * Function to get the objective function name from the reporter in the
   * optimizer.
   */
  PostprocessorName getObjectiveName() const;

  /**
   * Function to printout the domain mesh and information to a file
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of
   * the discrete Transfer class.
   */
  void printCurrentDomain(const dof_id_type & iteration);

  /**
   * Function to get the mesh pairs in the transfer and the allowed material to
   * optimize for
   */
  std::tuple<std::vector<subdomain_id_type> &,
             std::vector<subdomain_id_type> &,
             std::map<dof_id_type, subdomain_id_type> &>
  getMeshParameters();

  /**
   * Function to get the neighbors of an element using elements of the mesh.
   * @param[in] domain_mesh: the domain's mesh
   */
  void getNeighborsFromMesh(const MooseMesh & domain_mesh);

  /**
   * Template to convert a vector to a map (to the values part of a map). So
   * basically, the retturned map will have its values part (second) updated with
   * the vector content.
   * @param[in] keysMap: The map from which the keys will be used.
   * @param[in] values: The values vector to be assigned to the passed map
   */
  template <typename K, typename V>
  std::map<K, V> valuesVectorToMap(const std::map<K, V> & keysMap, const std::vector<V> & values)
  {
    if (keysMap.size() != values.size())
    {
      mooseError("Size of map and vector must be the same!");
    }

    std::map<K, V> resultMap;
    auto valueIt = values.begin();
    for (const auto & kv : keysMap)
    {
      resultMap[kv.first] = *valueIt; // get the current value from the vector
      ++valueIt;
    }

    return resultMap;
  }

  /**
   * Template to convert a map to its values vector. So basically, the
   * retturned vector will have the input map values part (second).
   * @param[in] inputMap: The map from which the values will be exctracted.
   */
  template <typename K, typename V>
  std::vector<V> mapToValuesVector(const std::map<K, V> & inputMap)
  {

    if (inputMap.empty())
    {
      mooseError("Input map must not be empty!");
    }

    std::vector<V> values;
    for (const auto & kv : inputMap)
    {
      values.push_back(kv.second);
    }
    return values;
  }

  /**
   * Function to print the domain's mesh.
   * @todo add the capability to have different Elements IDs instead of assuming
   * them constant.
   * @param[in] pairs_to_optimize: the current map that has the element and the
   * subdomain ids pairs to print.
   */
  void printMap(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize);

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************
  // No public variables to declare

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/

protected:
  //************************
  // Functions Declarations
  //************************

  virtual void initialize() override{};
  virtual void finalize() override{};
  // These objects are not threaded
  // void threadJoin(const UserObject &) final {}

  // /**
  //  * Function to check if new materials were added to the mesh
  //  */
  // bool isNewMaterialsInMesh(const MooseMesh & domain_mesh);

  /**
   * Function to get the optimization domain of the problem using the elements of
   * the mesh and their subdomain_ids.
   * @param[in] domain_mesh: the domain's mesh
   */
  void getOptimizationDomain(const MooseMesh & domain_mesh);

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************

  /// @brief Allowed values for my region_ID (e.g., materials possible to use {0,1,2}).
  std::vector<subdomain_id_type> _allowed_parameter_values;

  /// @brief excluded values for my region_ID (e.g., materials possible to use {0,1,2}).
  std::vector<subdomain_id_type> _excluded_parameter_values;

  /// @brief mapping between elements and subdomains. /// the current elem->subdomain assignment
  std::map<dof_id_type, subdomain_id_type> _pairs_to_optimize;

  /// @brief mapping between the element and its neighbors.
  std::map<dof_id_type, std::vector<dof_id_type>> _elem_neighbors;

  /// @brief The objective infromation as we got it from the discrete transfer, and to pass it to the optimizer
  Real _objective_result;

  /// @brief The objective name as we got it from the discrete transfer, and to pass it to the optimizer
  PostprocessorName _objective_name;

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/

private:
  //************************
  // Functions Declarations
  //************************
  // No private functions to declare

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************
  // No private variables to declare
};
