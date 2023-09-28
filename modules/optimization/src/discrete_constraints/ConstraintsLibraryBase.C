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

//*********************
// Regiester the MOOSE
//*********************
// registerMooseObject("OptimizationApp", ConstraintsLibraryBase);

//*******************************
// Adding validParams method
//*******************************
InputParameters
ConstraintsLibraryBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  return params;
}

void
ConstraintsLibraryBase::setConfiguration(const std::vector<int> & configuration)
{
  // This function is to set the configuration vector from the unrelated optimziation class. An
  // object of the child class will use this inherited function to setup the configuration. The
  // configuration is treated as a class variable (_configuration). It is then visible to other
  // child classes and one can start using it as required (say inside an execute method).

  // The configuration (without "_") will be passed by the optimziation module (e.g., the SA class)
  // through an object of the child class.

  // Making sure we clear the configuration member each time this function is used to set up the
  // configuration in the constraints library
  _configuration.clear();
  _configuration = configuration;

  // Now we extract the unique subdomain IDs. This assumes the configuration materials are not
  // changed during optimization.
  // {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 2, 3} configuration vector will be {0,1,2,3}.
  std::set<int> unique_material_ids(_configuration.begin(), _configuration.end());

  // excluded materials should be set before getting the unique materials IDs.
  if (_excluded_materials.empty())
  {
    mooseError("You have to set the excluded materials in your optimizer before getting the unique "
               "materials IDs! Use the setExcludedMaterials() method for this purpose while "
               "passing the excluded matrials vector to it.");
  }

  for (const auto & material : _excluded_materials)
  {
    unique_material_ids.erase(material);
  }

  _unique_material_ids.clear();
  _unique_material_ids = unique_material_ids;
}

void
ConstraintsLibraryBase::setPreviousConfiguration(const std::vector<int> & previous_configuration)
{
  _previous_configuration.clear();
  _previous_configuration = previous_configuration;
}

void
ConstraintsLibraryBase::setNeighborsMap(const std::map<int, std::vector<int>> & neighbors_map)
{

  // Setting up the excluded material to use.
  _neighbors_map.clear();
  _neighbors_map = neighbors_map;
}

void
ConstraintsLibraryBase::setExcludedMaterials(const std::vector<int> & exclude_values)
{

  // Setting up the excluded material to use.
  _excluded_materials.clear();
  _excluded_materials = exclude_values;
}

// void
// ConstraintsLibraryBase::setCellType(const int cell_type)
// {
//   _cell_type = cell_type;
// }

void
ConstraintsLibraryBase::setMeshDimension(const unsigned int dimension)
{
  _dimension = dimension;
}

//*******************
// Class Constructor
//*******************
ConstraintsLibraryBase::ConstraintsLibraryBase(const InputParameters & params)
  : GeneralUserObject(params)
{
}

//***********************
// Functions Definitions
//***********************
// No functions definitions as they are pure virtual
