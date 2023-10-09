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
#include "CustomOptimizationAlgorithm.h"
#include <fstream>
// #include "DiscreteConstraintsLibrary.h"

//*****************************
// Forward Declarations If Any
//*****************************
// class DiscreteConstraintsLibrary;

class SimulatedAnnealingAlgorithm : public CustomOptimizationAlgorithm
{
public:
  /**
   * Constructor.
   */
  SimulatedAnnealingAlgorithm();

  enum RealNeighborSelection
  {
    RandomDirectionStretching,
    BoxSampling
  };

  // void setConstraints(DiscreteConstraintsLibrary * constraints) override
  // {
  //   _constraints = constraints; // Set the member variable _constraints to the provided pointer
  // }

  /// purely virtual optimize function
  void solve() override;

  ///@{ public interface
  // Cooling & cooling() { return _cooling; }
  unsigned int & numSwaps() { return _num_swaps; }
  // std::vector<std::pair<Real, std::vector<int>>> & bestSolutions() { return best_solutions; }
  unsigned int & numReassignments() { return _num_reassignments; }
  void setValidReassignmentOptions(const std::set<int> & options);
  Real & relativePerturbationSize() { return _relative_perturbation; }
  void setLowerLimits(const std::vector<Real> & lower_limits);
  void setUpperLimits(const std::vector<Real> & upper_limits);
  RealNeighborSelection & realNeighborSelection() { return _real_perturbation_type; }
  ///@}

protected:
  /**
   * @brief Validates the parameters associated with the simulated annealing algorithm.
   *
   * Checks various conditions on the parameters set for the algorithm. If any of the conditions are
   * not met, an error is thrown indicating the specific issue. For instance, it ensures that the
   * solution size is not zero and checks certain conditions related to neighbor generation.
   *
   * @throws Throws a mooseError if any of the validation checks fail.
   */
  void validateParameters();

  /**
   * @brief Initializes the objective function value based on the initial configuration that has
   * been initialized/set in the executioner.
   *
   * This method fetches the objective function value and sets it to the "current_objective"
   * variable. It also updates the minimum objective of the algorithm with the current objective.
   *
   * @param [in,out] current_objective:: Reference to the current objective value. This will be
   * updated with the new objective value based on the current configuration.
   *
   * @return The updated objective value.
   */
  Real initializeObjective(Real & current_objective);

  /**
   * @brief Initialize the necessary variables and data structures for a new run of the simulated
   * annealing.
   *
   * This function resets counters, seeds random number generators, sets
   * initial temperatures, and configures the starting solutions based on the run number and
   * previously found solutions.
   *
   * @param [in] run: The current run number.
   * @param [in,out] current_objective: A reference to the current solution's objective. It might be
   * updated based on best solutions from previous runs.
   * @param [in,out] temp_current: A reference to the current temperature. It will be set to its
   * maximum value.
   * @param [in,out] solutions: A reference to a vector storing pairs of objective values and
   * associated integer solutions. Initialized for the current run.
   * @param [out] neighbor_int_configuration: A reference to the neighboring integer configuration.
   * Set based on current run.
   * @param [out] best_int_configuration: A reference to the best integer configuration. Set based
   * on current run.
   * @param [out] neighbor_real_configuration: A reference to the neighboring real configuration.
   * Set based on current run.
   * @param [out] best_real_configuration: A reference to the best real configuration. Set based on
   * current run.
   */
  void initializeRun(const unsigned int & run,
                     Real & current_objective,
                     Real & temp_current,
                     std::vector<std::pair<Real, std::vector<int>>> & best_solutions,
                     std::vector<int> & best_int_configuration,
                     std::vector<Real> & best_real_configuration);

  /**
   * @brief Generates neighbor configurations for the current state of the algorithm.
   *
   * Depending on the algorithm settings and the given run, this method generates a new
   * neighbor configuration for both real and integer configurations. For integer configurations,
   * it selects between combinatorial or standard methods based on the "_combinatorial_optimization"
   * flag.
   *
   * @param [in] run:: The current run iteration, used to influence the generation of neighbor
   * configurations.
   * @param [out] neighbor_int_configuration:: Reference to the integer configuration where the
   * generated neighbor integer configuration will be stored.
   * @param [out] neighbor_real_configuration:: Reference to the real configuration where the
   * generated neighbor real configuration will be stored.
   */
  void generateNeighborConfigurations(std::vector<int> & neighbor_int_configuration,
                                      std::vector<Real> & neighbor_real_configuration);

  /// creates neighbor states from current_states for the continuous params
  void createNeigborReal(const std::vector<Real> & real_sol, std::vector<Real> & real_neigh) const;

  /// random direction on the unit sphere
  void randomDirection(unsigned int size, std::vector<Real> & direction) const;

  /// creates neighbor states from current_states for the continuous params
  // void createNeigborInt(const std::vector<int> & int_sol, std::vector<int> & int_neigh) const;
  void createNeighborIntCombinatorial(const std::vector<int> & int_sol,
                                      std::vector<int> & int_neigh,
                                      const std::vector<int> & exclude_values) const;

  void createNeighborInt(const std::vector<int> & current_configuration,
                         std::vector<int> & neighbor_configuration,
                         const std::vector<int> & exclude_values) const;

  bool canFlipCombinatorial(const std::vector<int> & int_sol,
                            const std::vector<int> & int_neigh,
                            const std::map<int, std::vector<unsigned int>> & material_indices,
                            const unsigned int index1,
                            const unsigned int index2,
                            const std::map<int, std::vector<int>> & neighbors_map) const;

  bool canFlip(const std::vector<int> & int_sol,
               const unsigned int & index,
               const std::vector<int> & unique_material_ids,
               const int & new_val,
               const std::map<int, std::vector<int>> & neighbors_map) const;

  bool checkBoundaries(const std::vector<int> & int_vec,
                       const std::map<int, std::vector<int>> & neighborsMap) const;

  Real checkBoundingBoxDensity(const std::vector<int> & int_vec,
                               int cellType,
                               unsigned int dimension) const;

  unsigned int checkEnclaves(const std::vector<int> & int_vec,
                             int value,
                             const std::map<int, std::vector<int>> & neighborsMap) const;

  std::vector<int> getNeighbors(const std::map<int, std::vector<int>> & neighborsMap,
                                const int target_elem_id) const;

  bool acceptSolution(Real current_objective, Real neigh_objective, Real temp_current);

  /// computes the probability with which a neigbor objective value will be accepted given current objective value and temperature
  Real acceptProbability(Real curr_obj, Real neigh_obj, Real curr_temp) const;

  /**
   * @brief Handle logic related to an accepted solution in the simulated annealing process.
   *
   * This function updates the current solution's objective and configuration based on the provided
   * neighbor. It also manages the Tabu list by adding the current configuration to it and ensuring
   * it does not grow beyond its maximum size.
   *
   * @param [in,out] current_objective: A reference to the current solution's objective. Updated
   * with the neighbor's objective.
   * @param [in] neigh_objective: The objective of the neighbor solution.
   * @param [in] neighbor_int_configuration: A constant reference to the neighbor solution's integer
   * configuration.
   * @param [in] neighbor_real_configuration: A constant reference to the neighbor solution's real
   * configuration.
   * @param [in,out] tabu_list: A reference to the Tabu list. The current configuration will be
   * added to it, and old configurations might be removed.
   */
  void acceptedSolution(Real & current_objective,
                        const Real & neigh_objective,
                        const std::vector<int> & neighbor_int_configuration,
                        const std::vector<Real> & neighbor_real_configuration,
                        std::deque<std::vector<int>> & tabu_list);

  /// cooling schedule
  Real coolingSchedule(unsigned int step) const;

  /**
   * @brief Update the best solution found so far in the simulated annealing process.
   *
   * This function checks if the current solution's objective is better than the best known
   * objective and updates the best configuration and associated data if necessary.
   *
   * @param [in,out] current_objective: A reference to the current solution's objective.
   * @param [in] current_real_configuration_copy: A constant reference to the current
   * configuration's real values.
   * @param [in] current_int_configuration_copy: A constant reference to the current configuration's
   * integer values.
   * @param [out] best_real_configuration: A reference to the best configuration's real values.
   * @param [out] best_int_configuration: A reference to the best configuration's integer values.
   * @param [in,out] solutions: A reference to a vector storing pairs of objective values and
   * associated integer solutions.
   */
  void updateBestSolution(const Real & current_objective,
                          const std::vector<Real> & current_real_configuration_copy,
                          const std::vector<int> & current_int_configuration_copy,
                          std::vector<Real> & best_real_configuration,
                          std::vector<int> & best_int_configuration,
                          std::vector<std::pair<Real, std::vector<int>>> & solutions);

  /**
   * @brief Adjust the system temperature and revert to the best solution if necessary.
   *
   * This function performs a non-monotonic adjustment to the temperature if applicable.
   * It also checks if the temperature has dropped below a certain threshold and, if so,
   * reverts the current solution to the best known solution.
   *
   * @param [in,out] temp_current A reference to the current temperature. It might be adjusted
   * within this function.
   * @param [in,out] current_objective A reference to the current solution's objective. It might be
   * updated if the solution is reset.
   * @param [in] best_real_configuration A constant reference to the best solution's real values
   * used to reset the current configuration if needed.
   * @param [in] best_int_configuration A constant reference to the best solution's integer values
   * used to reset the current configuration if needed.
   */
  void coolAndReset(Real & temp_current,
                    Real & current_objective,
                    const std::vector<Real> & best_real_configuration,
                    const std::vector<int> & best_int_configuration);

  /**
   * @brief Store the best solution found after a run.
   *
   * This function updates the current configuration and objective based on the best solution found.
   * It also manages the list of best solutions, ensuring it remains sorted and contains only the
   * top solutions.
   *
   * @param [in,out] current_objective: Current objective value of the solution.
   * @param [in] best_real_configuration: The best real configuration found.
   * @param [in] best_int_configuration: The best integer configuration found.
   * @param [in,out] solutions: A vector storing pairs of objective values and associated integer
   * configurations.
   */
  void storeBestSolution(Real & current_objective,
                         const std::vector<Real> & best_real_configuration,
                         const std::vector<int> & best_int_configuration,
                         std::vector<std::pair<Real, std::vector<int>>> & best_solutions,
                         std::vector<std::pair<Real, std::vector<int>>> & solutions);

  void printBestSolutions(const std::vector<std::pair<Real, std::vector<int>>> & best_solutions);

  /// state size of the integer space
  unsigned int _int_state_size;

  /// state size of the real space
  unsigned int _real_state_size;

  /// alpha value for cooling
  Real _alpha;

  /// the best (aka min) objective seen so far
  Real _min_objective;

  /// cooling option
  // Cooling _cooling;

  /// if cooling is monotonic or not
  // bool _monotonic_cooling;

  /**
   * the temperature where simulated annealing starts resetting the current state
   * to the best found state. This temperature is halved every-time it is reached.
   */
  Real _res_var;

  ///@{ parameters governing the creation of neighbors for int params
  unsigned int _num_swaps;
  unsigned int _num_reassignments;
  std::vector<int> _valid_options;
  ///@}

  ///@{ parameters governing the creation of neighbors for real params
  RealNeighborSelection _real_perturbation_type;
  Real _relative_perturbation;
  bool _upper_limit_provided;
  bool _lower_limit_provided;
  std::vector<Real> _parameter_lower_limit;
  std::vector<Real> _parameter_upper_limit;
  ///@}

private:
  void logDebugInfo(const std::string & info);
  std::string vectorToString(const std::vector<int> & vec);
  std::string pairToString(const std::pair<Real, std::vector<int>> & p);
};
