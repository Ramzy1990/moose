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
#include "DiscreteConstraintsLibrary.h"

//*************************
// Helper Functions if Any
//*************************
// No helper methods to use
registerMooseObject("OptimizationApp", DiscreteConstraintsLibrary);

//*******************************
// Adding validParams method
//*******************************
InputParameters
DiscreteConstraintsLibrary::validParams()
{
  // InputParameters params = emptyInputParameters();
  InputParameters params = ConstraintsLibraryBase::validParams();

  params.addParam<bool>(
      "check_bounding_box_density",
      "This cosntraint checks the density of the bounding box around cells of each subdomain type. "
      "This helps in reducing fragmentations and tetris like configurations. ");

  // params.registerBase("DiscreteConstraints");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteConstraintsLibrary::DiscreteConstraintsLibrary(const InputParameters & params)
  : ConstraintsLibraryBase(params)
{

  // Debug variable, defaults to off
  _check_density =
      isParamValid("check_bounding_box_density") ? getParam<bool>("check_bounding_box_density") : 0;
}

//***********************
// Functions Definitions
//***********************

bool
DiscreteConstraintsLibrary::checkDensityImprovement(const std::vector<int> & old_solution,
                                                    const std::vector<int> & new_solution,
                                                    int cellType,
                                                    unsigned int dimension) const
{
  Real oldDensity = computeBoundingBoxDensity(old_solution, cellType, dimension);
  Real newDensity = computeBoundingBoxDensity(new_solution, cellType, dimension);
  return newDensity >= oldDensity;
}

Real
DiscreteConstraintsLibrary::computeBoundingBoxDensity(const std::vector<int> & int_vec,
                                                      int cellType,
                                                      unsigned int dimension) const
{
  unsigned int gridSize = static_cast<unsigned int>(pow(int_vec.size(), 1.0 / dimension));

  unsigned int min_i = gridSize, max_i = 0;
  unsigned int min_j = gridSize, max_j = 0;
  unsigned int min_k = gridSize, max_k = 0; // For 3D
  unsigned int cellCount = 0;

  for (unsigned int i = 0; i < gridSize; ++i)
  {
    for (unsigned int j = 0; j < gridSize; ++j)
    {
      for (unsigned int k = 0; k < (dimension == 3 ? gridSize : 1); ++k) // Loop only once for 2D
      {
        unsigned int index;
        if (dimension == 2)
        {
          index = i * gridSize + j;
        }
        else // 3D
        {
          index = i * gridSize * gridSize + j * gridSize + k;
        }

        if (int_vec[index] == cellType)
        {
          min_i = std::min(min_i, i);
          max_i = std::max(max_i, i);
          min_j = std::min(min_j, j);
          max_j = std::max(max_j, j);
          if (dimension == 3)
          {
            min_k = std::min(min_k, k);
            max_k = std::max(max_k, k);
          }
          cellCount++;
        }
      }
    }
  }

  unsigned int boundingBoxVolume;
  if (dimension == 2)
  {
    boundingBoxVolume = (max_i - min_i + 1) * (max_j - min_j + 1);
  }
  else // 3D
  {
    boundingBoxVolume = (max_i - min_i + 1) * (max_j - min_j + 1) * (max_k - min_k + 1);
  }

  if (boundingBoxVolume == 0)
    return 0; // Avoid division by zero

  return static_cast<Real>(cellCount) / boundingBoxVolume;
}
