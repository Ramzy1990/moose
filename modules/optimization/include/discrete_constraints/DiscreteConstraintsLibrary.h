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

class DiscreteConstraintsLibrary : public ConstraintsLibraryBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor and destructor.
   */
  DiscreteConstraintsLibrary(const InputParameters & parameters);
  // virtual ~DiscreteConstraintsUtils() = default;

  // Check if the density of the bounding box around cells of each type has improved
  bool checkDensityImprovement(const std::vector<int> & old_solution,
                               const std::vector<int> & new_solution,
                               int cellType,
                               unsigned int dimension) const;

  // Getter function to access _check_density
  bool shouldCheckDensity() const { return _check_density; }

protected:
  // Helper method to compute bounding box density
  Real computeBoundingBoxDensity(const std::vector<int> & int_vec,
                                 int cellType,
                                 unsigned int dimension) const;

  /// @brief variable for checking the density of the bounding box around cells of each type
  bool _check_density;
};

// unsigned int countEnclaves(const std::vector<int> & int_vec,
//                            int value,
//                            const std::map<int, std::vector<int>> & neighborsMap) const;

// bool canFlip(const std::vector<int> & int_vec,
//              unsigned int index,
//              unsigned int new_value,
//              const std::map<int, std::vector<int>> & neighborsMap) const;

// int chooseBoundaryElement(const std::map<int, std::vector<int>> & neighborsMap,
//                           const std::vector<int> & int_sol) const;

// bool isClusterRegular(const std::vector<int> & solution, int value) const;

// bool isValid(const std::vector<std::vector<int>> & grid) const;

// bool checkConstraints(const std::vector<int> & int_vec,
//                       const std::map<int, std::vector<int>> & neighborsMap) const;

// double computeBoundingBoxDensity(const std::vector<int> & int_vec,
//                                  int cellType,
//                                  unsigned int dimension) const;

// double computeBoundingBoxDensity2D(const std::vector<int> & int_vec, int cellType) const;
