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
#include "SolveObject.h"
#include "ExecFlagEnum.h"
#include "CustomOptimizationAlgorithm.h"

// Forward Declarations If Any
class DiscreteOptimizationReporter;
class DensityDiscreteConstraint;
class OptimizationReporterBase;
class SimulatedAnnealingAlgorithm;

/**
 * Contains methods for communicating between DiscreteOptimizationReporter and optimizers.
 */

class CustomOptimizeSolve : public SolveObject
{
public:
  //************************
  // Functions Declarations
  //************************

  static InputParameters validParams();
  CustomOptimizeSolve(Executioner & ex);

  virtual bool solve() override;

  OptimizationReporterBase & getOptimizationReporter() const { return *_obj_function; }

  // This way, we can use the funciton to get the class instance.
  DiscreteOptimizationReporter & getDiscreteOptimizationReporter() const { return *_reporter; }

  // Method to set the DensityDiscreteConstraint in the optimization algorithm
  void setDensityDiscreteConstraintForAlgorithm(const DensityDiscreteConstraint * ddc);

  // could be not needed!
  // DensityDiscreteConstraint & getDensityConstraint() { return *_ddc_uo; }

  // This way, we can use the funciton to get the class instance.
  // DiscreteConstraintsLibrary & getDiscreteConstraintsLibrary() const { return *_constraints; }

  // const std::vector<Real> & getRealParameters() const { return _real_parameters; }

  /// @brief Flag for if compinatorial_optimization is on or off
  bool _combinatorial_optimization;

  /// @brief The domains dimension
  unsigned int _dimension;

  /// @brief Flag for if the quarter symmetry is applied
  bool _quarter_symmetry;

  /// @brief Flag for if checking the domain's density is applied or not
  bool _check_density;

  /// @brief Flag for if compinatorial_optimization is on or off
  bool _check_enclaves;

  /// @brief Flag for if compinatorial_optimization is on or off
  bool _check_boundaries;

  /// @brief Flag for if compinatorial_optimization is on or off
  // bool _check_volumes;

  /// @brief The number of runs for this algorithm
  unsigned int _num_of_runs;

  /// @brief The number of iterations for this algorithm
  unsigned int _num_iterations;

  /// @brief The maximum temperature for this run
  Real _max_temp;

  /// @brief The minimum temperature for this run
  Real _min_temp;

  /// cooling option
  CustomOptimizationAlgorithm::Cooling _cooling;

  /// if cooling is monotonic or not
  bool _monotonic_cooling;

  /**
   * the temperature where simulated annealing starts resetting the current state
   * to the best found state. This temperature is halved every-time it is reached.
   */
  Real _res_var;

  /// @brief The debug flag
  bool _debug;

  //****************************************************************************************************************************//

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
  static void objectiveFunctionWrapper(Real & objective,
                                       const std::vector<Real> & rparams,
                                       const std::vector<int> & iparams,
                                       void * ctx);

  void updateTheApp();

  void print_table(std::string custom_optimizer_type,
                   //  std::string objective_function,
                   const unsigned int iteration,
                   const unsigned int run_iteration,
                   const PostprocessorName objective_name,
                   const Real objective_value,
                   bool solution_accepted,
                   bool tabu_list_used,
                   bool cache_used);

  //************************
  // Variables Declarations
  //************************

  /// @brief Object of the reporter class we are using
  DiscreteOptimizationReporter * _reporter;

  // /// @brief Object of the constraints class we are using
  // DiscreteConstraintsLibrary * _constraints;

  /// the optimization algorithm object
  std::unique_ptr<CustomOptimizationAlgorithm> _opt_alg;

  /// @brief Object of the density constraint class
  // DensityDiscreteConstraint * _ddc_uo;

  /// @brief Vector to hold the objective function value (assuming it has many components among different MultiApps)
  // std::vector<PostprocessorValue> & _objective_function_vector;

  /// @brief Variable to hold the objective function value
  // PostprocessorValue _objective_function_value;

  /// @brief Vector to hold the constraints results
  // std::vector<bool> _constraints_reults;

  /// @brief Vector to hold the domain's constraints results
  // std::vector<std::string> _domain_constraints;

  /// objective function defining objective, gradient, and hessian
  OptimizationReporterBase * _obj_function = nullptr;

  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// the optimization algorithm type
  MooseEnum _opt_alg_type;

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

private:
  //************************
  // Functions Declarations
  //************************

  //****************************************************************************************************************************//

  //************************
  // Variables Declarations
  //************************
  // Nothing to see here

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
};
