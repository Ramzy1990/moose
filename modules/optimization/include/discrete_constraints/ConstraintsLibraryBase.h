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
#include "GeneralUserObject.h"

//*****************************
// Forward Declarations If Any
//*****************************
// No forward declarations

class ConstraintsLibraryBase : public GeneralUserObject
{
public:
  //************************
  // Functions Declarations
  //************************

  static InputParameters validParams();

  /**
   * Constructor and destructor.
   */
  ConstraintsLibraryBase(const InputParameters & parameters);
  // virtual ~ConstraintsLibraryBase() = default;

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
  void setPreviousConfiguration(const std::vector<int> & previous_configuration);

  void setConfiguration(const std::vector<int> & configuration);

  void setNeighborsMap(const std::map<int, std::vector<int>> & neighbors_map);

  void setExcludedMaterials(const std::vector<int> & exclude_values);

  // void setCellType(const int cell_type);

  void setMeshDimension(const unsigned int dimension);

  // // Function to check the configuration with Neighbor
  // virtual bool checkConfigurationWithNeighbor(std::vector<int> & configuration,
  //                                             std::map<int, std::vector<int>> & neighbors_map,
  //                                             int cell_type,
  //                                             unsigned int dimension) = 0;

  // // Function to check the configuration without Neighbor
  // virtual bool checkConfigurationWithoutNeighbor(std::vector<int> & configuration,
  //                                                std::map<int, std::vector<int>> & neighbors_map,
  //                                                int cell_type,
  //                                                unsigned int dimension) = 0;

  virtual bool checkConfiguration() = 0;

  /*******************************************************************************************************************************/

  //************************
  // Variables Declarations
  //************************

  /// The configuration that other classes will see. They should be set using the optimziation class through an object of the
  /// child class
  std::vector<int> _configuration;

  /// The previous configuration to test against the current configuration. They should be set using the optimziation class through an object of the
  /// child class
  std::vector<int> _previous_configuration;

  /// The neighbors map, for each element and its neighbors. This is also seen by the child classes.
  std::map<int, std::vector<int>> _neighbors_map;

  /// The excluded materials information, which are those materials that are not treated for in the optimziation process and are
  /// fixed. This is also seen by the child classes.
  std::vector<int> _excluded_materials;
  bool _excluded_materials_assigned = false;

  /// The subdomain type (is it subdomain 0 or 1 or 2, etc.). This is also seen by the child classes.
  // int _cell_type;

  /// The mesh dimension (2D or 3D etc.). This is also seen by the child classes.
  unsigned int _dimension;

  /// The unique subdomain IDs in a configuration. This is also seen by the child classes. It is computed by default during the
  /// configuration setting to extract the material IDs we are dealing with.
  std::set<int> _unique_material_ids;

  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
  /*******************************************************************************************************************************/
};
