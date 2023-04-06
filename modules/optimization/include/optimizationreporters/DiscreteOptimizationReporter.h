//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// #include "ElementUserObject.h"
// #include "GeneralReporter.h"
#include "OptimizationReporter.h"
// #include "FEProblemBase.h"
// #include "OptimizationData.h"

/**
 * Contains reporters for communicating between optimizeSolveDiscrete and subapps. Methods
 * in OptimizeSolve is gradient-free (derivastive-free). Currently it includes TAO POUNDERS
 * and Nelder-Mead. It will eventually include Simulated Annealing and Genetic Algorithm.
 */
class DiscreteOptimizationReporter : // public ElementUserObject,
                                     //  public FEProblemBase,
                                    //  public GeneralReporter, // Included in OptimizationData
                                     public OptimizationReporter

{
public:
  static InputParameters validParams();

  DiscreteOptimizationReporter(const InputParameters & parameters);

  //************************
  // Variables Declarations
  //************************

  //************************
  // Functions Declarations
  //************************

  // void initialize() override {}
  // void execute() override {}
  // void threadJoin(const UserObject & /*uo*/); // since we include ElementUserObject.
  // void finalize() override {}
  // void initialSetup();

  /**
   * Function to check if the materials used are the ones included and allowed
   */
  void isMaterialAllowed(); // no need for pasing variables as we are checking the class ones.

  /**
   * Function to initialize the subdomain id vectors from input file
   */
  // void setInitialCondition(SubdomainName initial_material_used,
  //                          std::vector<SubdomainName> & initial_cell_subdomain_id,
  //                          dof_id_type total_cells);
  void setInitialCondition(std::vector<SubdomainName> & initial_cell_subdomain_id);

  /**
   * Function to set the targeted subdomain id vectors taking into account previous
   * cell_subdomain_id
   */
  void updateSubdomainID(const std::vector<SubdomainName> allowed_parameter_values,
                         const std::vector<SubdomainName> previous_cell_subdomain_id,
                         std::vector<SubdomainName> & cell_subdomain_id);

  /**
   * Function to compute the cost function
   */
  Real costFunction(const std::vector<SubdomainName> & Domain);

protected:
  //************************
  // Variables Declarations
  //************************

  /// The parameter names variable is a holder to the names of the variables we would
  /// like to use to control the optimization process. This is seen in the bimaterial test files (main.i). I guess we will
  /// need this one so I will keep it. E.g., parameter_names=materials.
  const std::vector<ReporterValueName> & _parameter_names;

  /// The number of variables is a holder to the number of variables adjusted under each
  /// parameter names we would like to use to control the optimization process. This is seen in the bimaterial test files
  /// (main.i). I guess we will need this one so I will keep it. E.g., num_values=2. (i.e.,
  /// two materials, fuel and moderator).
  /// Inherited from the OptimizationData or OptimizationReporter
  const std::vector<dof_id_type> & _nvalues;

  /// Number of parameter vectors. Might get handy if we would like to use.
  /// Inherited from the OptimizationData or OptimizationReporter
  unsigned int _nparam;

  /// Total number of parameters. Might get handy if we would like to use.
  /// Inherited from the OptimizationData or OptimizationReporter
  dof_id_type _ndof;

  /// Initial mateiral used
  SubdomainName _initial_material_used;

  /// ID assignment methodology.
  std::string _assign_type;

  /// Total number of cells. The 64-bit "dof_id_type" is much larger than the 16-bit "subdomain_id_type".
  dof_id_type _total_cells;

  /// Allowed values for my region_ID (e.g., materials possible to use {a,b,c}).
  std::vector<SubdomainName> _allowed_parameter_values;

  /// Subdomain ID Type. E.g., {a, b, c, d, etc...}
  std::vector<SubdomainName> _cell_subdomain_id;

  /// hold integer ID for each input pattern cell.
  // std::vector<dof_id_type> _cell_pattern;

  /// Initial or previous Subdomain ID Type. E.g., {a, b, c, d, etc...}
  std::vector<SubdomainName> _initial_cell_subdomain_id;

  /// From Element Subdomain Modifier.h:
  /// Any subdomain change is stored in this map. However, our subdomains IDs are not numbers
  /// but strings, and hence we need to use std::string
  // std::vector<std::pair<dof_id_type, SubdomainName>> _cached_subdomain_assignments;

  // Might become handy down the road, left as is for search purposes:
  // Map between RGMB element block names, block ids, and region ids
  // std::map<std::string, std::pair<subdomain_id_type, dof_id_type>> _name_id_map;

  //************************
  // Functions Declarations
  //************************

private:
  //************************
  // Functions Declarations
  //************************
};
