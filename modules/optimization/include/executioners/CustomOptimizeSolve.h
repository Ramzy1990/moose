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

// Forward Declarations If Any
class DiscreteOptimizationReporter;
class OptimizationReporterBase;
class CustomOptimizationAlgorithm;
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
  DiscreteOptimizationReporter & getDiscreteOptimizationReporter() const { return *_reporter; }
  // const std::vector<Real> & getRealParameters() const { return _real_parameters; }

  //
  //
  //
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

  static void objectiveFunctionWrapper(Real & objective,
                                       const std::vector<Real> & rparams,
                                       const std::vector<int> & iparams,
                                       void * ctx);
  //
  //
  //
  //
  //
  //
  //
  //************************
  // Variables Declarations
  //************************

  /// @brief Object of the reporter class we are using
  DiscreteOptimizationReporter * _reporter;

  /// @brief Vector to hold the objective function value (assuming it has many components among different MultiApps)
  // std::vector<PostprocessorValue> & _objective_function_vector;

  /// @brief Variable to hold the objective function value
  // PostprocessorValue _objective_function_value;

  /// @brief Vector to hold the constraints results
  std::vector<bool> _constraints_reults;

  /// @brief Vector to hold the domain's constraints results
  std::vector<std::string> _domain_constraints;

  /// objective function defining objective, gradient, and hessian
  OptimizationReporterBase * _obj_function = nullptr;

  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// the optimization algorithm type
  MooseEnum _opt_alg_type;

  /// the optimization algorithm object
  // std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<CustomOptimizationAlgorithm> _opt_alg;

  //****************************************************************************************************************************//
  //****************************************************************************************************************************//
  //****************************************************************************************************************************//

private:
  //************************
  // Functions Declarations
  //************************

  //
  //
  //
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
};
