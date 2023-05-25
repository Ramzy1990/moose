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
 * Contains reporters for communicating between optimizeSolveDiscrete and subapps. Methods
 * in OptimizeSolve is gradient-free (derivastive-free). Currently it includes TAO POUNDERS
 * and Nelder-Mead. It will eventually include Simulated Annealing and Genetic Algorithm.
 */
class DiscreteOptimizationReporter : public GeneralReporter
//  public ElementUserObject

{
public:
  //************************
  // Functions Declarations
  //************************
  static InputParameters validParams();

  DiscreteOptimizationReporter(const InputParameters & parameters);

  /**
   * Function to help in getting the domain's post processed results names.
   * @param[in] prefix: Use the supplied string as the prefix for vector postprocessor name
   * @param[in] pp_names: The post prcessors names as supplied from the subapps.
   * to print.
   */
  std::vector<VectorPostprocessorName>
  getVectorNamesHelper(const std::string & prefix, const std::vector<PostprocessorName> & pp_names);

  // void initialize() override {}
  // void execute() override {}
  // void threadJoin(const UserObject & /*uo*/); // since we include ElementUserObject.
  // void finalize() override {}
  // void initialSetup();

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
   * Function to bring in the post process results of the mesh to the reporter for it to pass it to
   * the optimizer.
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of the discrete
   * Transfer class.
   */
  void setDomainPostProcessInformation(const dof_id_type & iteration);

  /**
   * Function to printout the domain mesh and information to a file
   * @param[in] iteration: the current iteration in the FROM_MULTIAPP branch of the discrete
   * Transfer class.
   */
  void printCurrentDomain(const dof_id_type & iteration);

  /**
   * Function to update the mesh domain in the reporter based on the optimizer results. It is called
   * inside the optimizer.
   * @todo add the capability to have different Elements IDs instead of assuming them constant. Name
   * change to updateMeshIds.
   * @param[in] pairs_to_optimize: the current map that has the element and the subdomain ids pairs
   * to optimize.
   * This pairs_to_optimize will be eventually sent to the TO_MULTIAPP branch of the discrete
   * transfer class to assigna nd update the mesh.
   *
   */
  void updateMeshDomain(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize);

  /**
   * Functions to get the mesh pairs to optimize and the allowed material to optimize for
   */
  std::tuple<std::vector<subdomain_id_type> &,
             std::map<dof_id_type, subdomain_id_type> &,
             std::map<dof_id_type, subdomain_id_type> &>
  getMeshParameters();

  //
  //
  //
  //
  //************************
  // Variables Declarations
  //************************
  // Nothing to see here

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

protected:
  //************************
  // Functions Declarations
  //************************

  virtual void initialize() override{};
  void execute();
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

  /**
   * Function to print the domain's mesh.
   * @todo add the capability to have different Elements IDs instead of assuming them constant.
   * @param[in] pairs_to_optimize: the current map that has the element and the subdomain ids pairs
   * to print.
   */
  void printMap(const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize);

  //
  //
  //
  //
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

  /// Allowed values for my region_ID (e.g., materials possible to use {a,b,c}).
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

  /// pairing between subdomains_ids and their names
  /// use getSubdomainNames in a function with ->first and ->second
  // std::map<SubdomainID, SubdomainName> _subdomain_id_to_name;

  /// From Element Subdomain Modifier.h:
  /// Any subdomain change is stored in this map. However, our subdomains IDs are not numbers
  /// but strings, and hence we need to use std::string
  // std::vector<std::pair<dof_id_type, SubdomainName>> _cached_subdomain_assignments;

  // Might become handy down the road, left as is for search purposes:
  // Map between RGMB element block names, block ids, and region ids
  // std::map<std::string, std::pair<subdomain_id_type, dof_id_type>> _name_id_map;

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

private:
  //************************
  // Functions Declarations
  //************************
  // Nothing to see here

  //
  //
  //
  //
  //************************
  // Variables Declarations
  //************************
  // Nothing to see here
};
