//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Note: For Doxygen, variables have comments starting with "///", while the functions
// (methods) or classes has the "/**" comment before it.
// General comments uses "//"

// Okay, so OptimizationData is a reporter as well! So we have in general 4 reporters in
// the optimization module:
// OptimizationData
// OptimizationInfo
// OptimizationReporter
// OptimizationReporterBase

// The general shape of an app in MOOSE, according to the files structure: Actions, Base, Kernels,
// Executioners, Functions, Reporters, Transfers, User Objects, and Postprocessing. Are we
// supposed to make a shape/discrete optimization module that is totally not dependednt on the
// inverse optimization module? That is an idea to start with and is plausible.
// If we are going to do so, then the FEProblemBase is not needed here, as well as the
// ElementUserObject, since they wiould be included in the main shape optimization app I guess.

// I think it is getting clearer now!

// Inherits from the ElementUserObject and the FEProblemBase. If we would like to add another, just
// use ",".
// Inherits from the Main optimization module, through OptimizationData, some objects and most
// importantly the GeneralReporter.h.
#include "ElementUserObject.h"
// #include "FEProblemBase.h"
#include "OptimizationData.h"

// The optimization reporter has two header files. One seems to contain the methods
// (OptimizationReporterBase) and the other seems to contain the variables (OptimizationReporter).
// OptimizationReporterBase inherits from OptimizationData.
// OptimizationReporter inherits from the OptimizationReporterBase.
// Why?!
// Anyway, our DiscreteOptimizationReporter will just have one and will then inherits from
// the OptimizationData. we will also use the computeObjective function from there.

// if "ElementUserObject.h" is included here, there is no need to include it in the source file.

/**
 * Contains reporters for communicating between optimizeSolveDiscrete and subapps. Methods
 * in OptimizeSolve is gradient-free (derivastive-free). Currently it includes TAO POUNDERS
 * and Nelder-Mead. It will eventually include Simulated Annealing and Genetic Algorithm.
 */
class DiscreteOptimizationReporter : public ElementUserObject,
                                     //  public FEProblemBase,
                                     //  public GeneralReporter, Included in OptimizationData
                                     public OptimizationData
{
public:
  // Always here as seen from other classes developed in MOOSE.
  static InputParameters validParams();

  // Always here as seen from other classes developed in MOOSE.
  DiscreteOptimizationReporter(const InputParameters & parameters);

  /// Methods used decalred in this header file
  /// Some add virtual, some do not.
  /// @{ Block all methods that are not used in explicitly called UOs
  void initialize() override{};
  void execute() override;
  void threadJoin(const UserObject & /*uo*/) override{}; // since we include ElementUserObject.
  void finalize() override;
  void initialSetup() override;
  /// @}

protected:
  // Methods/variables protected

  // Here, the variables for this class are declared, but then their definitions are done in the
  // source file. Note that the name is matching, and some of them are assigned with the
  // "getParam" getter (a function/method), which might be related to the input file.

  /// Subdomain ID Type. The Materials in the 2D mesh. E.g., {a, b, c, d, etc...}
  /// This is one of the vectors we optimize for. It should be a vector of vectors.
  /// We have a simialr one for the elements ID.
  std::vector<std::vector<std::string>> _region_id;

  /// Name of Subdomain ID, just in case we would like to identify the subdomain ID letters with material names.
  const std::vector<std::string> _region_id_name;

  /// hold integer ID for each input pattern cell. Eventually, it will be upgraded to "Elem" type for FEM.
  /// "dof_id_type" is the data type, unsigned 64-bit integer.
  /// Cell Pattern is a vector of vectors with different number of elements in different rows.
  /// Example: _cell_pattern{ { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9, 4 } };
  /// Another example would be : _cell_pattern {{1},{2},{3},{4},{5},{6},{7},{8},{9}, ... }
  /// Another example would be : _cell_pattern {{1,2,3,4,5,6,7,8,9}, ... }. The choice, I
  /// persume, depends on column/row order majors in the memory (I am not sure if
  /// std::vector follows C++ row major ordering).
  std::vector<std::vector<dof_id_type>> _cell_pattern;

  // Cells whose subdomain IDs have changed
  std::vector<std::vector<dof_id_type>> _changed_cell_pattern;

  /// integer ID assignment type.
  /// For testing this discrete class.
  /// Manual where we assign manually the cells pattern.
  /// Automatic where I really do not know how, but I have a hint!!
  const std::string _assign_type;

  /// Allowed values for my region_ID (e.g., materials possible to use {a,b,c}).
  /// The "set" here is instead of type "vector".
  /// A set in C++ is a container that store unique elements following a specific order (or
  /// it can be unordered as well).
  std::set<std::string> _allowed_parameter_values;

  /// Boolian to see if the used region ID is within the allowed values.
  const bool _is_allowed;

  /// From Element Subdomain Modifier.h:
  /// Any subdomain change is stored in this map. However, our subdomains IDs are not numbers
  /// but strings, and hence we need to use std::string
  std::vector<std::pair<dof_id_type, std::string>> _cached_subdomain_assignments;

  /// Total number of cells. The 64-bit "dof_id_type" is much larger than the 16-bit "subdomain_id_type".
  const dof_id_type _total_cells;

  // Might become handy down the road, left as is for search purposes:
  // Map between RGMB element block names, block ids, and region ids
  // std::map<std::string, std::pair<subdomain_id_type, dof_id_type>> _name_id_map;

  /**
   * Function to compute the objective function value (here function is like mathematical
   * function, mind you!). The real computation happens in the OptimizeSolve source though, and
   * this function is actually called to call that main solver. This is the last function
   * called in objective routine. Value returned is real.
   */
  virtual Real computeObjective();
};
