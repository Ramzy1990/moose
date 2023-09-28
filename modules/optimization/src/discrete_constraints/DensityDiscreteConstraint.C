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
#include "DensityDiscreteConstraint.h"

//*************************
// Helper Functions if Any
//*************************
// No helper methods to use

//*********************
// Regiester the MOOSE
//*********************
registerMooseObject("OptimizationApp", DensityDiscreteConstraint);

//*******************************
// Adding validParams method
//*******************************
InputParameters
DensityDiscreteConstraint::validParams()
{
  // InputParameters params = emptyInputParameters();
  InputParameters params = ConstraintsLibraryBase::validParams();

  params.addParam<bool>(
      "check_bounding_box_density",
      "This cosntraint checks the density of the bounding box around cells of each subdomain type. "
      "This helps in reducing fragmentations and tetris like configurations. ");

  return params;
}

//*******************
// Class Constructor
//*******************
DensityDiscreteConstraint::DensityDiscreteConstraint(const InputParameters & params)
  : ConstraintsLibraryBase(params)
{

  // check density variable, defaults to off
  _check_density =
      isParamValid("check_bounding_box_density") ? getParam<bool>("check_bounding_box_density") : 0;
}

//***********************
// Functions Definitions
//***********************

void
DensityDiscreteConstraint::execute()
{
}

bool
DensityDiscreteConstraint::checkConfiguration()
{
  // Check density improvement
  // The check can be here or it can be in the optimizer, where the shouldCheckDensity() method
  // is used to get the value of _check_density.
  bool improved_density = true;
  for (const auto & material : _unique_material_ids)
  {
    Real current_density = checkBoundingBoxDensity(_previous_configuration, material, _dimension);
    Real new_density = checkBoundingBoxDensity(_configuration, material, _dimension);

    if (new_density < current_density)
    {
      improved_density = false;
      break; // As soon as we find a material that does not satisfy the condition, break.
    }
  }

  return improved_density;
}

Real
DensityDiscreteConstraint::checkBoundingBoxDensity(const std::vector<int> & int_vec,
                                                   int cell_type,
                                                   unsigned int dimension)
{
  unsigned int grid_size = static_cast<unsigned int>(pow(int_vec.size(), 1.0 / dimension));

  unsigned int min_i = grid_size, max_i = 0;
  unsigned int min_j = grid_size, max_j = 0;
  unsigned int min_k = grid_size, max_k = 0; // For 3D
  unsigned int cell_count = 0;

  for (unsigned int i = 0; i < grid_size; ++i)
  {
    for (unsigned int j = 0; j < grid_size; ++j)
    {
      for (unsigned int k = 0; k < (dimension == 3 ? grid_size : 1); ++k) // Loop only once for 2D
      {
        unsigned int index;
        if (dimension == 2)
        {
          index = i * grid_size + j;
        }
        else // 3D
        {
          index = i * grid_size * grid_size + j * grid_size + k;
        }

        if (int_vec[index] == cell_type)
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
          cell_count++;
        }
      }
    }
  }

  unsigned int bounding_box_volume;
  if (dimension == 2)
  {
    bounding_box_volume = (max_i - min_i + 1) * (max_j - min_j + 1);
  }
  else // 3D
  {
    bounding_box_volume = (max_i - min_i + 1) * (max_j - min_j + 1) * (max_k - min_k + 1);
  }

  if (bounding_box_volume == 0)
    return 0; // Avoid division by zero

  return static_cast<Real>(cell_count) / bounding_box_volume;
}
