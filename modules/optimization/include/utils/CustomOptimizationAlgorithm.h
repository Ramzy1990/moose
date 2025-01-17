//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"
#include "MooseRandom.h"

//*****************************
// Forward Declarations If Any
//*****************************
class DensityDiscreteConstraint;

class CustomOptimizationAlgorithm
{
public:
  /**
   * Constructor.
   */
  CustomOptimizationAlgorithm();
  virtual ~CustomOptimizationAlgorithm() = default; // destructor?

  /// sets the objective routine
  void setObjectiveRoutine(void (*fncptr)(Real & objective,
                                          const std::vector<Real> & rparams,
                                          const std::vector<int> & iparams,
                                          void * ctx),
                           void * ctx)
  {
    _ctx = ctx;
    objectiveFunction = fncptr;
  };

  /// purely virtual optimize function
  virtual void solve() = 0;

  ///@{ public interface
  void setInitialSolution(const std::vector<Real> & real_sol,
                          const std::vector<int> & int_sol,
                          const std::vector<int> & exclude_materials,
                          const std::map<int, std::vector<int>> & elem_neighbors);

  /// cooling enum
  enum Cooling
  {
    LinMult,
    ExpMult,
    LogMult,
    QuadMult,
    LinAdd,
    QuadAdd,
    ExpAdd,
    TrigAdd,
    trial
  };

  // virtual void setConstraints(DiscreteConstraintsLibrary * constraints)
  // {
  //   mooseError("setConstraints should not be called on a CustomOptimizationAlgorithm object! "
  //              "Please override it in the derived class (e.g., in
  //              SimulatedAnnealingAlgorithm).");
  // }

  void setSeed(unsigned int seed);

  void setDensityDiscreteConstraint(const DensityDiscreteConstraint * ddc) { _ddc_ptr = ddc; }

  bool & combinatorialOptimization() { return _combinatorial_optimization; }
  unsigned int & meshDimesnsion() { return _dimension; }
  bool & quarterSymmetry() { return _quarter_symmetry; }
  bool & checkDensityConstraint() { return _check_density; }
  bool & checkEnclavesConstraint() { return _check_enclaves; }
  bool & checkBoundariesConstraint() { return _check_boundaries; }
  unsigned int & maxIt() { return _max_its; }
  unsigned int & maxRun() { return _num_runs; }
  Real & maxTemp() { return _temp_max; }
  Real & minTemp() { return _temp_min; }
  Cooling & coolingEquation() { return _cooling; }
  bool & monotonicCooling() { return _monotonic_cooling; }
  Real & resTemp() { return _temp_res; }
  bool & debug() { return _debug_on; }
  unsigned int & counterIteration() { return _it_counter; }
  unsigned int & counterRun() { return _it_run; }
  const std::vector<Real> & realSolution() const { return _current_real_config; }
  std::vector<int> & intSolution() { return _current_int_config; }
  Real & objective() { return _objective_value; }
  bool & tabu() { return _tabu_used; }
  bool & cache() { return _cache_used; }
  bool & solution() { return _solution_accepted; }

  ///@}

  std::vector<int> _current_int_config;
  std::vector<int> _exclude_materials;
  std::map<int, std::vector<int>> _elem_neighbors;

  /// the best (aka min) objective seen so far
  Real _objective_value;

protected:
  void (*objectiveFunction)(Real & objective,
                            const std::vector<Real> & rparams,
                            const std::vector<int> & iparams,
                            void * ctx);
  void * _ctx;

  /// @brief pointer to the density discrete constraint object
  const DensityDiscreteConstraint * _ddc_ptr = nullptr;

  /// @brief Flag for if compinatorial_optimization is on or off
  bool _combinatorial_optimization;

  /// @brief The domains dimension
  unsigned int _dimension;

  /// @brief Flag for if the quarter symmetry is applied
  bool _quarter_symmetry;

  /// @brief Flag for if checking the domain's density is applied or not
  bool _check_density;

  /// @brief Flag for if checking the enclaves is applied or not
  bool _check_enclaves;

  /// @brief Flag for if checking the materials beside the boundaries is applied or not
  bool _check_boundaries;

  /// maximum number of steps/iterations
  unsigned int _max_its;

  /// maximum number of runs
  unsigned int _num_runs;

  /// maximum temperature
  Real _temp_max;

  /// minimum temperature
  Real _temp_min;

  /// cooling schedule
  Cooling _cooling;

  /// monotonic cooling
  bool _monotonic_cooling;

  /// Restart temperature
  Real _temp_res;

  /// debug flag
  bool _debug_on;

  /// iteration counter
  unsigned int _it_counter;

  /// iteration of multiruns
  unsigned int _it_run;

  /// Tabu list used
  bool _tabu_used;

  /// cache used
  bool _cache_used;

  /// solution accepted
  bool _solution_accepted;

  /// a random seed in case randomization is used for optimization algorithm
  unsigned int _random_seed;

  ///@{ the current solution
  std::vector<Real> _current_real_config;

  unsigned int _real_size;
  unsigned int _int_size;
  unsigned int _size;
  ///@}
};
