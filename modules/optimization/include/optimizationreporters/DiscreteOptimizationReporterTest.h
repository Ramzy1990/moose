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

/**
 * Contains reporters for communicating between DiscreteOptimizeSolve and subapps for testing
 * purposes.
 */
class DiscreteOptimizationReporterTest : public GeneralReporter
//  public ElementUserObject

{
public:
  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  // Constructor
  DiscreteOptimizationReporterTest(const InputParameters & parameters);

  void execute() override {}

  /**
   * Function to check if the materials used are the ones included and allowed
   */
  void isMaterialAllowed(const MooseMesh & domain_mesh);

  /**
   * Function to initialize the subdomain id vectors
   */
  void setInitialCondition(const MooseMesh & domain_mesh);

  /**
   * Function to update the optimization domain based on some logic, used for testing purposes.
   */
  void updateSubdomainID(const std::vector<subdomain_id_type> allowed_parameter_values,
                         std::map<dof_id_type, subdomain_id_type> & previous_pairs_to_optimize,
                         std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize,
                         const MooseMesh & domain_mesh);

  /**
   * Function to printout the domain mesh and information to a file
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of the discrete
   * Transfer class.
   */
  void printCurrentDomain(const dof_id_type & iteration);

  /**
   * Functions to get the mesh pairs to optimize and the allowed material to optimize for
   */
  std::tuple<std::vector<subdomain_id_type> &,
             std::map<dof_id_type, subdomain_id_type> &,
             std::map<dof_id_type, subdomain_id_type> &>
  getMeshParameters();

  /**
   * Template to convert a vector to a map (to the values part of a map). So basically, the
   * retturned map will have its values part (second) updated with the vector content.
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

    // std::map<K, V> resultMap;
    // std::size_t i = 0;
    // for (const auto& kv : keysMap) {
    //     resultMap[kv.first] = values[i];
    //     ++i;
    // }

    // return resultMap;
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
   * @todo add the capability to have different Elements IDs instead of assuming them constant.
   * @param[in] pairs_to_optimize: the current map that has the element and the subdomain ids pairs
   * to print.
   */
  void printMap(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize);

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************
  // No variables to declare

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/

protected:
  //************************
  // Functions Declarations
  //************************

  virtual void initialize() override{};
  // void execute();
  virtual void finalize() override{};
  // These objects are not threaded
  // void threadJoin(const UserObject &) final {}
  // /**
  //  * Function to check if new materials were added to the mesh
  //  */
  // bool isNewMaterialsInMesh(const MooseMesh & domain_mesh);

  /**
   * Function to get the optimization domain of the problem using the elements of the mesh
   * and their subdomain_ids.
   */
  void getOptimizationDomain(const MooseMesh & domain_mesh);

  /**
   * Function to update the optimization domain based on some logic, used for testing purposes.
   */
  void
  testUpdateSubdomainID(const std::vector<subdomain_id_type> allowed_parameter_values,
                        const std::map<dof_id_type, subdomain_id_type> previous_pairs_to_optimize,
                        std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize);

  /**
   * Function to compute the cost function
   */
  unsigned int costFunctionTest(const MooseMesh & domain_mesh,
                                const std::map<dof_id_type, subdomain_id_type> & domain_map);

  /**
   * Function to compute the cost function in SubApps
   */
  unsigned int costFunction(const MooseMesh & domain_mesh,
                            const std::map<dof_id_type, subdomain_id_type> & domain_map);

  // void updateMeshIds(const dof_id_type & iteration); // when we have both element and domain ids
  // changing.

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************

  ///  Getting the mesh. General reporter has _fe_problem.
  // MooseMesh & moose_mesh = problem.mesh();
  // MeshBase & mesh = moose_mesh.getMesh();
  MooseMesh & _mesh;

  /// Initial mateiral used
  subdomain_id_type _initial_material_used;

  /// ID assignment methodology.
  std::string _assign_type;

  /// Solver methodology.
  std::string _solve_type;

  /// Total number of cells. The 64-bit "dof_id_type" is much larger than the 16-bit "subdomain_id_type".
  dof_id_type _total_cells;

  /// Allowed values for my region_ID (e.g., materials possible to use {0,1,2}).
  std::vector<subdomain_id_type> _allowed_parameter_values;

  /// Initial or previous elements. E.g., {0, 1, 1000, 500, etc...}
  std::vector<dof_id_type> _initial_elements_to_optimize;

  /// Initial or previous Subdomain ID. E.g., {0, 1, 2, 3, etc...}
  std::vector<subdomain_id_type> _initial_subdomains_to_optimize;

  /// Initial or previous pairs. E.g., {{0,1}, {0,2}, etc...}
  std::map<dof_id_type, subdomain_id_type> _initial_pairs_to_optimize;

  /// elements of our mesh we would like to optimize
  std::vector<dof_id_type> _elements_to_optimize; // From ElementSubdomainIDGenerator source. = elem

  /// subdomains of our mesh we would like to optimize
  std::vector<subdomain_id_type> _subdomains_to_optimize; // = elem.subdomain_id()

  /// mapping between elements and subdomains. /// the current elem->subdomain assignment
  std::map<dof_id_type, subdomain_id_type> _pairs_to_optimize;

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
