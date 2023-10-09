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
#include "SimulatedAnnealingAlgorithm.h"
#include "DensityDiscreteConstraint.h"
#include <iomanip> // Include for std::setprecision
#include <limits>
#include <queue>
#include <fstream>
#include <filesystem> // Include for std::filesystem

//*********************
// Regiester the MOOSE
//*********************
// No regiester to carry

//*************************
// Helper Functions if Any
//*************************
// The vector_hash struct
// Taken from: https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
// Used to hash the solutions cache with a map!
// Not needed is solution cache is not used.
struct vector_hash
{
  template <class T1>
  std::size_t operator()(const std::vector<T1> & vector) const
  {
    std::size_t seed = vector.size();
    for (auto & i : vector)
    {
      seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

//*******************
// Class Constructor
//*******************
SimulatedAnnealingAlgorithm::SimulatedAnnealingAlgorithm()
  : CustomOptimizationAlgorithm(),
    _alpha(1e-2),
    _min_objective(std::numeric_limits<Real>::max()),
    _num_swaps(1),
    _num_reassignments(0),
    _real_perturbation_type(RandomDirectionStretching),
    _relative_perturbation(0.05),
    _upper_limit_provided(false),
    _lower_limit_provided(false)
{
}

void
SimulatedAnnealingAlgorithm::solve()
{
  // Some basic checks to validate
  validateParameters();

  std::vector<std::pair<Real, std::vector<int>>> best_solutions; // Store best solutions of each run

  Real current_objective;                 /*current objective*/
  Real temp_current;                      /*current temperature*/
  std::deque<std::vector<int>> tabu_list; /*the tabu list, any configuration inside will result
  in
                                             continuing the while loop*/
  // Solution cache
  // std::unordered_map<std::vector<int>, Real, vector_hash> solution_cache;

  initializeObjective(current_objective);

  std::vector<Real> neighbor_real_config;
  std::vector<int> neighbor_int_config;
  std::vector<Real> best_real_config;
  std::vector<int> best_int_config;
  int tabu_skip_counter = 0; // to counter the stagnation problems seen

  std::unordered_map<std::vector<int>, Real, vector_hash> solution_cache;

  for (int run = 0; run < _num_runs; ++run)
  {

    initializeRun(
        run, current_objective, temp_current, best_solutions, best_int_config, best_real_config);

    MooseRandom::seed(std::time(0) + run); // New seed for each run.

    // std::vector<std::pair<Real, std::vector<int>>> solutions(
    //     3, std::make_pair(std::numeric_limits<Real>::max(), std::vector<int>()));
    std::vector<std::pair<Real, std::vector<int>>> solutions;
    // Starting the main Annealing loop
    while (_it_counter < _max_its && std::abs(temp_current) > _temp_min)
    {
      // Sets the seed for the random number generator
      // MooseRandom::seed(std::time(0) + run); // HUGE PERFORMANCE HIT INSIDE WHILE LOOP!!!!!

      if (_debug_on)
      {
        logDebugInfo("==== START OF ITERATION: " + std::to_string(_it_counter) + " ====");
      }
      if (_debug_on)
      {
        logDebugInfo("Cooling Equation used is of Type: " + std::to_string(_cooling));
        logDebugInfo("Monotonic Cooling is being used : " + std::to_string(_monotonic_cooling));
      }
      if (_debug_on)
      {
        logDebugInfo("Maximum temperature is: " + std::to_string(_temp_max));
        logDebugInfo("minimum temperature is: " + std::to_string(_temp_min));
        logDebugInfo("Restart temperature is: " + std::to_string(_temp_res));
      }

      // Generates the neighbor configurations
      generateNeighborConfigurations(neighbor_int_config, neighbor_real_config);

      // Tabu list is only enabled for integer configurations
      _tabu_used = false;
      if (tabu_skip_counter > 7)
      {
        tabu_list.pop_front(); // Remove the oldest entry
        // tabu_list.clear(); // Clear the entire tabu list if it is already quite small
        tabu_skip_counter = 0; // Reset the counter
      }
      if (std::find(tabu_list.begin(), tabu_list.end(), neighbor_int_config) != tabu_list.end())
      {
        _tabu_used = true;
        tabu_skip_counter++;
        continue;
      }

      // Now generating the objective value assocaited with the generated neighbor configuration
      _cache_used = false;
      Real neigh_objective;
      // auto it = solution_cache.find(neighbor_int_config);
      // if (it != solution_cache.end())
      // {
      //   // If the solution is in the cache, use the cached objective value
      //   neigh_objective = it->second;
      //   _cache_used = true;
      // }
      // else
      // {
      objectiveFunction(neigh_objective, neighbor_real_config, neighbor_int_config, _ctx);
      // solution_cache[neighbor_int_config] = neigh_objective;
      // }

      if (_debug_on)
      {
        logDebugInfo("Neighbor's objective value: " + std::to_string(neigh_objective));
        logDebugInfo("Generated neighbor_int_config used in the objective function: " +
                     vectorToString(neighbor_int_config));
      }

      std::vector<int> current_int_solution_copy = _current_int_config;

      // acceptance check: lower objective always accepted;
      // acceptance check: lower temps always accepted;
      // higher objective sometimes accepted
      // All those are propabilities as seen in the acceptProbability() function
      // This is why we sometimes have a bad objective function after a good one
      _solution_accepted = false;
      Real temp_r = MooseRandom::rand();
      if (temp_r <= acceptProbability(current_objective, neigh_objective, temp_current))
      {

        if (_debug_on)
        {
          logDebugInfo(
              "acceptProbability() method result: " +
              std::to_string(acceptProbability(current_objective, neigh_objective, temp_current)));
          logDebugInfo("Random value for acceptance check (temp_r): " + std::to_string(temp_r));
        }

        // if we accept then it always counts as a new step
        ++_it_counter;

        _solution_accepted = true;
        acceptedSolution(current_objective,
                         neigh_objective,
                         neighbor_int_config,
                         neighbor_real_config,
                         tabu_list);
      }
      else
      {
        // otherwise, it has a 10% chance to count as a new step to finish the problem
        // this is especially important for combinatorial problems
        Real temp_rr = MooseRandom::rand();
        if (temp_rr <= 0.1)
          ++_it_counter;
      }

      if (_debug_on)
        logDebugInfo("temp_current after the solution acceptance check: " +
                     std::to_string(temp_current));

      // cool the temperature
      temp_current = coolingSchedule(_it_counter);

      if (_debug_on)
        logDebugInfo("temp_current after temperature cooling: " + std::to_string(temp_current));

      // if this is the best objective function, it is our new best value
      if (current_objective < _min_objective)
      {
        updateBestSolution(current_objective,
                           neighbor_real_config,
                           neighbor_int_config,
                           best_real_config,
                           best_int_config,
                           solutions);
      }

      coolAndReset(temp_current, current_objective, best_real_config, best_int_config);

      if (_debug_on)
      {
        logDebugInfo("==== END OF ITERATION: " + std::to_string(_it_counter - 1) + " ====");
        logDebugInfo("============================================");
      }

    } // While loop ending

    if (_debug_on)
      logDebugInfo("End of run " + std::to_string(run) +
                   ". Final _min_objective: " + std::to_string(_min_objective));

    storeBestSolution(
        current_objective, best_real_config, best_int_config, best_solutions, solutions);

    if (_debug_on)
    {
      // Logging at the end of the run
      logDebugInfo("==== END OF RUN: " + std::to_string(run) + " ====");
      logDebugInfo("Final current_objective for this run: " + std::to_string(current_objective));
      logDebugInfo("Final _current_int_config for this run: " +
                   vectorToString(_current_int_config));
      logDebugInfo("Neighbor_int_config at end of this run: " +
                   vectorToString(neighbor_int_config));

      logDebugInfo("Final best_int_config for this run: " + vectorToString(best_int_config));

      logDebugInfo("Best solutions at the end of this run:");
      for (const auto & solution : best_solutions)
      {
        logDebugInfo(pairToString(solution));
      }

      logDebugInfo("============================================");
    }
  }

  printBestSolutions(best_solutions);
}

void
SimulatedAnnealingAlgorithm::validateParameters()
{
  if (_size == 0)
    mooseError("Solution size is zero. Most likely setInitialSolution was not called.");

  // ... other validation checks ...
  if (_int_state_size > 0 && _num_reassignments > 0 && _valid_options.size() < 2)
    mooseError("If the number of reassignments for neighbor generation is > 0, then the number of "
               "valid options must be at least 2.");

  if (_int_state_size > 0 && _num_swaps + _num_reassignments == 0)
    mooseError("The problem has a non-zero number of categorical parameters, but the number of "
               "swaps and number of reassignments for neighbor generation are both 0.");
}

Real
SimulatedAnnealingAlgorithm::initializeObjective(Real & current_objective)
{

  // Get the objective function value and put it in the "current_objective" variable.
  // Here we basically compute the initial minimum objective depending on the initial
  // configurations.
  objectiveFunction(current_objective, _current_real_config, _current_int_config, _ctx);

  // Set the minimum objective to the current objective.
  _min_objective = current_objective;

  // Get the updated objective.
  return current_objective;
}

void
SimulatedAnnealingAlgorithm::initializeRun(
    const unsigned int & run,
    Real & current_objective,
    Real & temp_current,
    std::vector<std::pair<Real, std::vector<int>>> & best_solutions,
    std::vector<int> & best_int_config,
    std::vector<Real> & best_real_config)
{

  // initialization code for each run

  // The run iterator
  _it_run = run;

  // the number of "accepted" steps [not necessarily equal to # of traversals of while loop because
  // of not accepting solutions sometimes and the probability involved]
  _it_counter = 0;

  /*the initial temperature to start resetting the current state to the best found
                         state when this temperature is reached. This temperature is halved
     every-time it is reached.*/
  _res_var = 1.0;

  // Current temperature is set to maximum one.
  temp_current = _temp_max;

  // Real values are not adjusted yet
  best_real_config = _current_real_config;

  // Initialize solutions
  if (run == 0)
  {
    // Objective was initialized already in the initializeObjective() method
    best_int_config = _current_int_config;
  }
  else if (run > 0)
  {
    current_objective =
        best_solutions.front().first;   // Update the current objective with the objective value
                                        // from the best solution of the previous runs
    _min_objective = current_objective; // Minimum objective is e_best in the fortran code

    best_int_config = best_solutions.front().second;
    _current_int_config = best_solutions.front().second;
  }

  if (_debug_on)
  {
    logDebugInfo("==== START OF RUN: " + std::to_string(run) + " ====");
    logDebugInfo("Initial temp_current: " + std::to_string(temp_current));
    logDebugInfo("Initial current_objective: " + std::to_string(current_objective));
    logDebugInfo("Best configuration found: " + vectorToString(best_int_config));
    logDebugInfo("Initial current configuration for this run: " +
                 vectorToString(_current_int_config));
  }
}

void
SimulatedAnnealingAlgorithm::generateNeighborConfigurations(
    std::vector<int> & neighbor_int_config, std::vector<Real> & neighbor_real_config)
{
  createNeigborReal(_current_real_config, neighbor_real_config);

  if (_combinatorial_optimization)
  {
    createNeighborIntCombinatorial(_current_int_config, neighbor_int_config, _exclude_materials);
  }
  else
  {
    createNeighborInt(_current_int_config, neighbor_int_config, _exclude_materials);
  }

  if (_debug_on)
  {
    logDebugInfo("Generated neighbor_int_config: " + vectorToString(neighbor_int_config));
  }
}

void
SimulatedAnnealingAlgorithm::createNeigborReal(const std::vector<Real> & real_sol,
                                               std::vector<Real> & real_neigh) const
{
  // Sets the seed for the random number generator
  // MooseRandom::seed(std::time(0) + run);

  if (_real_size == 0)
  {
    real_neigh = {};
    return;
  }

  Real norm = 0;
  for (auto & p : real_sol)
    norm += p * p;
  norm = std::sqrt(norm);

  switch (_real_perturbation_type)
  {
    case RandomDirectionStretching:
      // we need to loop until we find a perturbation
      // that does not violate the parameter limits
      while (true)
      {
        Real norm_perturbation = norm * _relative_perturbation * MooseRandom::randNormal(0.0, 1.0);
        std::vector<Real> dir;
        randomDirection(_real_size, dir);

        bool accept = true;
        for (unsigned int j = 0; j < _real_size; ++j)
        {
          real_neigh[j] = real_sol[j] + norm_perturbation * dir[j];

          if (_upper_limit_provided && real_neigh[j] > _parameter_upper_limit[j])
            accept = false;

          if (_lower_limit_provided && real_neigh[j] < _parameter_lower_limit[j])
            accept = false;

          if (!accept)
            break;
        }

        if (accept)
          return;
      }
      break;
    case BoxSampling:
      if (!_upper_limit_provided || !_lower_limit_provided)
        mooseError("Upper and lower limits must be provided for BoxSampling");

      for (unsigned int j = 0; j < _real_size; ++j)
        real_neigh[j] =
            _parameter_lower_limit[j] +
            MooseRandom::rand() * (_parameter_upper_limit[j] - _parameter_lower_limit[j]);
      break;
    default:
      mooseError("Unrecognized neigbor perturation selection");
  }
}

// Muller & Marsaglia (‘Normalized Gaussians’)
void
SimulatedAnnealingAlgorithm::randomDirection(unsigned int size, std::vector<Real> & direction) const
{
  direction.resize(size);

  // set direction to a vector normally distributed random variables
  for (unsigned int j = 0; j < size; ++j)
    direction[j] = MooseRandom::randNormal(0.0, 1.0);

  // compute the norm of direction
  Real norm = 0.0;
  for (auto & p : direction)
    norm += p * p;
  norm = std::sqrt(norm);

  // normalize
  for (unsigned int j = 0; j < size; ++j)
    direction[j] /= norm;
}

void
SimulatedAnnealingAlgorithm::createNeighborIntCombinatorial(
    const std::vector<int> & int_sol,
    std::vector<int> & int_neigh,
    const std::vector<int> & exclude_values) const
{

  // Sets the seed for the random number generator
  // MooseRandom::seed(std::time(0) + run);

  int_neigh = int_sol; // Start with a copy of the current configuration

  // Create a map of material IDs to their indices
  std::map<int, std::vector<unsigned int>> material_indices;

  for (unsigned int i = 0; i < int_neigh.size(); ++i)
  {
    if (std::find(exclude_values.begin(), exclude_values.end(), int_neigh[i]) ==
        exclude_values.end())
    {
      material_indices[int_neigh[i]].push_back(i);
    }
  }

  // Ensure there are at least two materials to flip
  if (material_indices.size() < 2)
  {
    mooseError(
        "The createNeighborInt() method received just one material in the whole domain! Please "
        "make sure you are optimizing a domain that contains at least 2 materials!");
  }

  const unsigned int max_attempts = 100; // Limit the number of flip attempts
  unsigned int attempts = 0;

  const unsigned int max_canflip_calls = 100; // Limit the number of calls to canFlip
  unsigned int canflip_calls = 0;

  // If no valid flipping is found, we basically return to the original int_sol we started with.
  bool valid_flip_found = false;

  while (attempts < max_attempts)
  {
    unsigned int index1 = MooseRandom::randl() % int_neigh.size();
    unsigned int index2 = index1;

    while (index1 == index2 ||
           std::find(exclude_values.begin(), exclude_values.end(), int_neigh[index1]) !=
               exclude_values.end() ||
           std::find(exclude_values.begin(), exclude_values.end(), int_neigh[index2]) !=
               exclude_values.end())
    {
      index2 = MooseRandom::randl() % int_neigh.size();
      index1 = MooseRandom::randl() % int_neigh.size();
    }

    if (int_neigh[index1] != int_neigh[index2])
    {
      std::swap(int_neigh[index1], int_neigh[index2]);
    }
    else
    {
      continue;
    }

    if (canFlipCombinatorial(int_sol, int_neigh, material_indices, index1, index2, _elem_neighbors))
    {
      valid_flip_found = true;
      break; // Exit the loop if a valid configuration is found
    }
    else
    {
      // If not valid, revert the changes and try again
      // int_neigh = int_sol;
      std::swap(int_neigh[index1], int_neigh[index2]);
      attempts++;

      canflip_calls++;
      if (canflip_calls >= max_canflip_calls)
      {
        break; // Exit the loop if we have called canFlip too many times
      }
    }

    // If no valid flip was found after all attempts, revert int_neigh to int_sol
    if (!valid_flip_found)
    {
      int_neigh = int_sol;
    }
  }
}

void
SimulatedAnnealingAlgorithm::createNeighborInt(const std::vector<int> & current_configuration,
                                               std::vector<int> & neighbor_configuration,
                                               const std::vector<int> & exclude_values) const
{

  ///////////////////////////////////////////////////////////
  //// Many vlaues but flipping just one element at a time///
  ///////////////////////////////////////////////////////////

  // std::cout << "Starting createNeighborInt()" << std::endl;

  if (_int_size == 0)
  {
    // std::cout << "_int_size is 0, clearing neighbor_configuration and returning." << std::endl;
    neighbor_configuration.clear();
    return;
  }

  neighbor_configuration = current_configuration;

  std::set<int> unique_material_ids_set;
  for (const auto & id : current_configuration)
  {
    if (std::find(exclude_values.begin(), exclude_values.end(), id) == exclude_values.end())
    {
      unique_material_ids_set.insert(id);
    }
  }

  // std::cout << "Number of unique material IDs (excluding the given values): "
  //           << unique_material_ids_set.size() << std::endl;

  std::vector<int> unique_material_ids(unique_material_ids_set.begin(),
                                       unique_material_ids_set.end());

  if (unique_material_ids.size() < 2)
  {
    mooseError("Please optimize a domain with at least 2 materials!");
    return;
  }

  unsigned int selected_index;
  do
  {
    selected_index = MooseRandom::randl() % _int_size;
  } while (std::find(exclude_values.begin(),
                     exclude_values.end(),
                     neighbor_configuration[selected_index]) != exclude_values.end());

  // std::cout << "Selected index for flipping: " << selected_index << std::endl;
  // std::cout << "Value at selected index (" << selected_index
  //           << "): " << neighbor_configuration[selected_index] << std::endl;

  int min_value = *std::min_element(current_configuration.begin(), current_configuration.end());
  int max_value = *std::max_element(current_configuration.begin(), current_configuration.end());
  // std::cout << "Min value: " << min_value << ", Max value: " << max_value << std::endl;

  const int max_attempts = 10;
  int attempt_count = 0;
  int replacement_value = 0;

  // All the possible material replacements to choose from are the ones included in the unique
  // materials IDs. But if the current index has the same material we are going to replace, this
  // will not change the outcome, then we will have to remove it from the possible replacemnts.
  std::vector<int> possible_replacements = unique_material_ids;
  possible_replacements.erase(std::remove(possible_replacements.begin(),
                                          possible_replacements.end(),
                                          neighbor_configuration[selected_index]),
                              possible_replacements.end());

  // std::cout << "Possible replacements: ";
  // for (int val : possible_replacements)
  // {
  //   std::cout << val << " ";
  // }
  // std::cout << std::endl;

  bool valid_replacement_found = false;

  do
  {
    int random_index = MooseRandom::randl() % possible_replacements.size();
    replacement_value = possible_replacements[random_index];

    // std::cout << "Generated replacement value: " << replacement_value
    //           << std::endl; // Logging the generated value

    bool is_value_same = replacement_value == neighbor_configuration[selected_index];
    bool is_excluded = std::find(exclude_values.begin(), exclude_values.end(), replacement_value) !=
                       exclude_values.end();
    bool is_canflip_false = !canFlip(current_configuration,
                                     selected_index,
                                     unique_material_ids,
                                     replacement_value,
                                     _elem_neighbors);

    // std::cout << "Is value the same? " << is_value_same << std::endl;
    // std::cout << "Is value in excluded list? " << is_excluded << std::endl;
    // std::cout << "Is canFlip returning false? " << is_canflip_false << std::endl;

    bool is_invalid_replacement = is_value_same || is_excluded || is_canflip_false;

    if (!is_invalid_replacement)
    {
      valid_replacement_found = true;
    }
    else
    {
      // std::cout << "Attempt #" << attempt_count + 1 << ": Replacement value " <<
      // replacement_value
      //           << " is invalid. Retrying..." << std::endl;
      attempt_count++;
    }

  } while (!valid_replacement_found && attempt_count < max_attempts);

  if (valid_replacement_found)
  {
    neighbor_configuration[selected_index] = replacement_value;
    // std::cout << "Successfully replaced value at index " << selected_index << " with "
    //           << replacement_value << std::endl;
  }
  else
  {
    // std::cout << "Failed to find a valid replacement value after " << max_attempts
    //           << " attempts. Keeping original value." << std::endl;
  }

  // Checking if any of the starting values is missing or not
  std::vector<bool> has_value(max_value - min_value + 1, false);
  for (unsigned int i = 0; i < _int_size; ++i)
  {
    has_value[neighbor_configuration[i] - min_value] = true;
  }

  for (int i = 0; i <= max_value - min_value; ++i)
  {
    if (!has_value[i] && std::find(exclude_values.begin(), exclude_values.end(), i + min_value) ==
                             exclude_values.end())
    {
      int random_index;
      do
      {
        random_index = MooseRandom::randl() % _int_size;
      } while (std::find(exclude_values.begin(),
                         exclude_values.end(),
                         neighbor_configuration[random_index]) != exclude_values.end() ||
               !canFlip(current_configuration,
                        random_index,
                        unique_material_ids,
                        i + min_value,
                        _elem_neighbors));

      neighbor_configuration[random_index] = i + min_value;
      // std::cout << "Inserted missing value " << i + min_value << " at index " << random_index
      //           << std::endl;
      break;
    }
  }

  // std::cout << "Exiting createNeighborInt()" << std::endl;

  //   ///////////////////////////////////////////////////////////
  //   /////////////       One and two values           //////////
  //   ///////////////////////////////////////////////////////////

  // if (_int_size == 0)
  // {
  //   int_neigh = {};
  //   return;
  // }
  // unsigned int index = MooseRandom::randl() % _int_size;
  // int_neigh = int_sol;
  // // Flip the value at the chosen index (change 1 to 2 or vice versa)
  // if (int_neigh[index] == 1)
  // {
  //   int_neigh[index] = 2;
  // }
  // else
  // {
  //   int_neigh[index] = 1;
  // }

  // // Check to make sure the domain contains at least one type of each material
  // bool hasOne = false, hasTwo = false;
  // for (unsigned int i = 0; i < _int_size; ++i)
  // {
  //   if (int_neigh[i] == 1)
  //   {
  //     hasOne = true;
  //   }
  //   else if (int_neigh[i] == 2)
  //   {
  //     hasTwo = true;
  //   }

  //   // If both materials are found, break the loop early
  //   if (hasOne && hasTwo)
  //     break;
  // }

  // // If either material is missing, insert it at a random position
  // if (!hasOne || !hasTwo)
  // {
  //   int_neigh[MooseRandom::randl() % _int_size] = !hasOne ? 1 : 2;
  // }

  //   ///////////////////////////////////////////////////////////
  //   ///////// Many vlaues but flipping a lot at a time ////////
  //   ///////////////////////////////////////////////////////////
  // if (_int_size == 0)
  // {
  //   int_neigh = {};
  //   return;
  // }

  // Real flip_ratio = 0.80; // Adjust this to the desired flip ratio
  // unsigned int num_flips = static_cast<unsigned int>(_int_size * flip_ratio);
  // if (num_flips > _int_size)
  // {
  //   num_flips = _int_size; // ensure we do not try to flip more elements than we have
  // }

  // // Get the min and max values from int_sol
  // subdomain_id_type min_value = *std::min_element(int_sol.begin(), int_sol.end());
  // subdomain_id_type max_value = *std::max_element(int_sol.begin(), int_sol.end());

  // int_neigh = int_sol; // copy the configuration into the neighbor

  // for (unsigned int flip = 0; flip < num_flips; ++flip)
  // {
  //   unsigned int index;
  //   // Find a random index whose value is not in exclude_values
  //   do
  //   {
  //     index = MooseRandom::randl() % _int_size;
  //   } while (std::find(exclude_values.begin(), exclude_values.end(), int_neigh[index]) !=
  //            exclude_values.end());

  //   // Generate a new random value different from the current one and not in exclude_values
  //   subdomain_id_type new_val;
  //   do
  //   {
  //     // Adjust the modulus operation to be the size of our desired range (max_value - min_value +
  //     // 1) Add the minimum value, min_value, to shift this range up from starting at 0 to starting
  //     // at our minimum value.
  //     new_val = MooseRandom::randl() % (max_value - min_value + 1) + min_value;
  //   } while (new_val == int_neigh[index] ||
  //            std::find(exclude_values.begin(), exclude_values.end(), new_val) !=
  //                exclude_values.end());

  //   int_neigh[index] = new_val; // assign the new value
  // }
}

bool
SimulatedAnnealingAlgorithm::canFlipCombinatorial(
    const std::vector<int> & int_sol,
    const std::vector<int> & int_neigh,
    const std::map<int, std::vector<unsigned int>> & material_indices,
    const unsigned int index1,
    const unsigned int index2,
    const std::map<int, std::vector<int>> & neighbors_map) const
{
  // Check the boundary constraints
  if (_check_boundaries)
  {
    if (!checkBoundaries(int_neigh, neighbors_map))
      return false;
  }

  // Now, check the previous constraints regarding the number of enclaves
  // Assuming you have a method to count enclaves for a given material
  if (_check_enclaves)
  {
    for (const auto & [material, indices] : material_indices)
    {
      // unsigned int enclave_count = checkEnclaves(int_neigh, material, neighbors_map);
      unsigned int count0 = checkEnclaves(int_neigh, 0, neighbors_map);
      unsigned int count1 = checkEnclaves(int_neigh, 1, neighbors_map);
      if (count0 > 2 || count1 > 1)
        // Adjust the condition based on the allowed number of enclaves for each material
        // if (enclave_count > /* some threshold based on material */)
        return false;
    }
  }

  // Check density improvement through the constraint library for the density!
  // We have to first set the different parameters needed in the constraint library.
  // if (_ddc_ptr->shouldCheckDensity())
  // {
  //   _ddc_ptr->setExcludedMaterials(_exclude_materials);
  //   _ddc_ptr->setPreviousConfiguration(int_neigh);
  //   _ddc_ptr->setConfiguration(int_sol);
  //   _ddc_ptr->setNeighborsMap(neighbors_map);
  //   // _ddc_ptr->setCellType(int_sol); // not needed here
  // _ddc_ptr->setMeshDimension(_dimension);

  //   // With those set up, now we can call to check the density
  // return _ddc_ptr->checkConfiguration();
  // }
  if (_check_density)
  {
    bool improved_density = true;
    for (const auto & [material, indices] : material_indices)
    {
      Real current_density = checkBoundingBoxDensity(int_sol, material, _dimension);
      Real new_density = checkBoundingBoxDensity(int_neigh, material, _dimension);

      if (new_density < current_density)
      {
        improved_density = false;
        break;
      }
    }

    if (!improved_density)
    {
      return false; // Skip the rest of the method iteration
    }
  }

  // // Now, check the previous constraints regarding the number of enclaves
  // if (_check_enclaves)
  // {
  //   unsigned int count0 = checkEnclaves(int_neigh, 0, neighbors_map);
  //   unsigned int count1 = checkEnclaves(int_neigh, 1, neighbors_map);

  //   if (count0 > 2 || count1 > 1)
  //     return false;
  // }

  return true;
}

bool
SimulatedAnnealingAlgorithm::canFlip(const std::vector<int> & int_sol,
                                     const unsigned int & index,
                                     const std::vector<int> & unique_material_ids,
                                     const int & new_val,
                                     const std::map<int, std::vector<int>> & neighbors_map) const
{
  // std::cout << "_check_density: " << _check_density << ", _check_boundaries: " <<
  // _check_boundaries
  //           << ", _check_enclaves: " << _check_enclaves << std::endl;

  // Create a test vector with the proposed flip
  std::vector<int> testVec = int_sol;
  testVec[index] = new_val;

  // Check density improvement through the constraint library for the density!
  // We have to first set the different parameters needed in the constraint library.
  // if (_ddc_ptr->shouldCheckDensity())
  // {
  //   _ddc_ptr->setExcludedMaterials(_exclude_materials); // always first to decalre, or at least
  //                                                       // before the setConfiguration() method.
  //   _ddc_ptr->setPreviousConfiguration(int_sol);
  //   _ddc_ptr->setConfiguration(testVec); // testVec has an element changed to new_val.
  //   _ddc_ptr->setNeighborsMap(neighbors_map);
  //   _ddc_ptr->setMeshDimension(_dimension);

  //   // With those set up, now we can call to check the density
  //   return _ddc_ptr->checkConfiguration();
  // }

  // Check density improvement
  if (_check_density)
  {
    bool improved_density = true;
    for (const auto & material : unique_material_ids)
    {
      Real current_density = checkBoundingBoxDensity(int_sol, material, _dimension);
      Real new_density = checkBoundingBoxDensity(testVec, material, _dimension);

      if (new_density < current_density)
      {
        improved_density = false;
        break;
      }
    }

    if (!improved_density)
    {
      return false; // Skip the rest of the method iteration
    }
  }

  // check the boundaries
  if (_check_boundaries)
  {
    if (!checkBoundaries(testVec, neighbors_map))
      return false;
  }

  // Now, check the previous constraints regarding the number of enclaves
  if (_check_enclaves)
  {
    // TODO: once the constraint library is added, this will be handeled there.
    unsigned int count0 = checkEnclaves(testVec, 0, neighbors_map);
    unsigned int count1 = checkEnclaves(testVec, 1, neighbors_map);

    if (count0 > 2 || count1 > 1)
      return false;
  }

  // Add other constraints as necessary

  return true; // The flip is valid
}

bool
SimulatedAnnealingAlgorithm::checkBoundaries(
    const std::vector<int> & int_vec, const std::map<int, std::vector<int>> & neighborsMap) const
{
  unsigned int grid_size = static_cast<unsigned int>(sqrt(int_vec.size()));
  unsigned int cube_length = static_cast<unsigned int>(cbrt(int_vec.size()));

  auto idx3D = [&](unsigned int x, unsigned int y, unsigned int z) -> unsigned int
  { return (cube_length - 1 - z) * cube_length * cube_length + y * cube_length + x; };

  // // Check if the bottom-right corner has a 0s region
  // if (int_vec[(grid_size - 1) * grid_size + (grid_size - 1)] != 0)
  // {
  //   return false; // Constraint not met
  // }

  // // Check if the bottom boundary has fuel touching
  // for (unsigned int j = 0; j < grid_size; ++j)
  // {
  //   if (int_vec[(grid_size - 1) * grid_size + j] != 1)
  //   {
  //     return false; // Constraint not met
  //   }
  // }

  // // Check if the right boundary has fuel touching
  // for (unsigned int i = 0; i < grid_size; ++i)
  // {
  //   if (int_vec[i * grid_size + (grid_size - 1)] != 1)
  //   {
  //     return false; // Constraint not met
  //   }
  // }

  // // Ensure the top boundary doesn't have any fuel touching
  // for (unsigned int j = 0; j < grid_size; ++j)
  // {
  //   if (int_vec[j] == 1)
  //   {
  //     return false; // Constraint not met
  //   }
  // }

  // // Ensure the left boundary doesn't have any fuel touching
  // for (unsigned int i = 0; i < grid_size; ++i)
  // {
  //   if (int_vec[i * grid_size] == 1)
  //   {
  //     return false; // Constraint not met
  //   }
  // }
  if (_dimension == 2)
  {
    // bool zerosAtBottomLeft = (int_vec[(grid_size - 1) * grid_size] == 0);
    // if (!zerosAtBottomLeft)
    //   return false;

    bool fuelTouchesBottom = false;
    bool fuelTouchesLeft = false;
    for (unsigned int i = 0; i < grid_size; ++i)
    {
      if (int_vec[(grid_size - 1) * grid_size + i] == 1)
        fuelTouchesBottom = true;
      if (int_vec[i * grid_size] == 1)
        fuelTouchesLeft = true;
    }
    if (!fuelTouchesBottom || !fuelTouchesLeft)
      return false;

    bool fuelTouchesTop = false;
    bool fuelTouchesRight = false;
    for (unsigned int i = 0; i < grid_size; ++i)
    {
      if (int_vec[i] == 1)
        fuelTouchesTop = true;
      if (int_vec[i * grid_size + (grid_size - 1)] == 1)
        fuelTouchesRight = true;
    }
    if (fuelTouchesTop || fuelTouchesRight)
      return false;
  }

  else if (_dimension == 3)
  {

    bool zerosAtBottomLeft = true;

    for (unsigned int z = 0; z < cube_length; ++z)
    {
      if (int_vec[idx3D(0, cube_length - 1, z)] !=
          0) // (0, cube_length - 1) denotes the bottom-left in each layer
      {
        zerosAtBottomLeft = false;
        break;
      }
    }

    if (!zerosAtBottomLeft)
      return false;

    bool westFaceTouched = false;
    bool southFaceTouched = false;

    // West Face
    for (unsigned int z = 0; z < cube_length; ++z)
    {
      for (unsigned int y = 0; y < cube_length; ++y)
      {
        if (int_vec[idx3D(0, y, z)] == 1)
        {
          westFaceTouched = true;
          break; // Move to the next layer
        }
      }
      if (westFaceTouched)
        break; // Exit the outer loop if constraint met
    }

    // South Face (touching 1s)
    for (unsigned int z = 0; z < cube_length; ++z)
    {
      for (unsigned int x = 0; x < cube_length; ++x)
      {
        if (int_vec[idx3D(x, cube_length - 1, z)] == 1)
        {
          southFaceTouched = true;
          break; // Move to the next layer
        }
      }
      if (southFaceTouched)
        break; // Exit the outer loop if constraint met
    }

    // Top Face (all 0s)
    for (unsigned int z = 0; z < cube_length; ++z)
    {
      for (unsigned int x = 0; x < cube_length; ++x)
      {
        if (int_vec[idx3D(x, 0, z)] != 0)
        {
          return false; // Constraint not met
        }
      }
    }

    // Right Face
    for (unsigned int z = 0; z < cube_length; ++z)
    {
      for (unsigned int y = 0; y < cube_length; ++y)
      {
        if (int_vec[idx3D(cube_length - 1, y, z)] != 0)
        {
          return false; // Constraint not met
        }
      }
    }

    // After all constraints are checked
    return westFaceTouched && southFaceTouched;
  }

  // 3. Check that the main fuel enclave is continuous
  // Using BFS for this purpose
  std::vector<bool> visited(int_vec.size(), false);
  std::queue<int> queue;
  int start_fuel_index =
      std::find(int_vec.begin(), int_vec.end(), 1) - int_vec.begin(); // Find the first fuel cell
  queue.push(start_fuel_index);
  visited[start_fuel_index] = true;

  while (!queue.empty())
  {
    int current = queue.front();
    queue.pop();
    for (int neighbor : neighborsMap.at(current))
    {
      if (int_vec[neighbor] == 1 && !visited[neighbor])
      {
        queue.push(neighbor);
        visited[neighbor] = true;
      }
    }
  }

  // If any fuel cell is not visited, then the enclave is not continuous
  for (unsigned int i = 0; i < int_vec.size(); ++i)
  {
    if (int_vec[i] == 1 && !visited[i])
      return false;
  }

  // // For the water enclave starting from top-left (if any)
  // std::vector<bool> water_visited(int_vec.size(), false);
  // std::queue<int> water_queue;

  // int top_left_index = (grid_size - 1) * grid_size; // Top-left corner index

  // if (int_vec[top_left_index] == 0) // If top-left is water
  // {
  //   water_queue.push(top_left_index);
  //   water_visited[top_left_index] = true;

  //   while (!water_queue.empty())
  //   {
  //     int current = water_queue.front();
  //     water_queue.pop();
  //     for (int neighbor : neighborsMap.at(current))
  //     {
  //       if (int_vec[neighbor] == 0 && !water_visited[neighbor])
  //       {
  //         water_queue.push(neighbor);
  //         water_visited[neighbor] = true;
  //       }
  //     }
  //   }
  // }

  // // Check the total count of the water cells
  // unsigned int fuel_cells = std::count(int_vec.begin(), int_vec.end(), 1);
  // unsigned int water_cells_visited = std::count(water_visited.begin(), water_visited.end(),
  // true); if (water_cells_visited != int_vec.size() - fuel_cells)
  //   return false;

  return true;
}

Real
SimulatedAnnealingAlgorithm::checkBoundingBoxDensity(const std::vector<int> & int_vec,
                                                     int cell_type,
                                                     unsigned int dimension) const
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

unsigned int
SimulatedAnnealingAlgorithm::checkEnclaves(
    const std::vector<int> & int_vec,
    int value,
    const std::map<int, std::vector<int>> & neighbors_map) const
{
  std::vector<int> visited(int_vec.size(), 0);
  unsigned int regions = 0;
  unsigned int max_regions = (value == 0) ? 2 : 1;

  // std::cout << "Starting checkEnclaves for value: " << value << "\n";

  for (unsigned int i = 0; i < int_vec.size() && regions <= max_regions; ++i)
  {
    if (int_vec[i] == value && visited[i] == 0)
    {
      // std::cout << "Found a new region at index: " << i << "\n";

      std::queue<int> queue;
      queue.push(i);
      visited[i] = 1;
      regions++;

      if (regions > max_regions)
      {
        // std::cout << "Regions exceeded max allowed regions. Exiting...\n";
        return regions;
      }

      while (!queue.empty())
      {
        int current = queue.front();
        queue.pop();

        for (int neighbor : getNeighbors(neighbors_map, current))
        {
          if (int_vec[neighbor] == value && visited[neighbor] == 0)
          {
            queue.push(neighbor);
            visited[neighbor] = 1;
          }
        }
      }
    }
  }

  // std::cout << "Total regions for value " << value << ": " << regions << "\n";
  return regions;
}

std::vector<int>
SimulatedAnnealingAlgorithm::getNeighbors(const std::map<int, std::vector<int>> & neighbors_map,
                                          const int target_elem_id) const
{
  // If the element exists in the map, return its neighbors; otherwise, return an empty vector.
  if (neighbors_map.find(target_elem_id) != neighbors_map.end())
  {
    return neighbors_map.at(target_elem_id);
  }
  else
  {
    return std::vector<int>();
  }
}

bool
SimulatedAnnealingAlgorithm::acceptSolution(Real current_objective,
                                            Real neigh_objective,
                                            Real temp_current)
{
  Real temp_r = MooseRandom::rand();

  if (temp_r <= acceptProbability(current_objective, neigh_objective, temp_current))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void
SimulatedAnnealingAlgorithm::acceptedSolution(Real & current_objective,
                                              const Real & neigh_objective,
                                              const std::vector<int> & neighbor_int_config,
                                              const std::vector<Real> & neighbor_real_config,
                                              std::deque<std::vector<int>> & tabu_list)
{
  // We assign the members to the generated configuration (configuration/objective)
  _current_real_config = neighbor_real_config;
  _current_int_config = neighbor_int_config;
  current_objective = neigh_objective;

  // Add the current configuration to the Tabu list
  tabu_list.push_back(_current_int_config);

  // If the Tabu list is too big, remove the oldest element
  if (tabu_list.size() > 100)
  {
    tabu_list.pop_front();
  }

  if (_debug_on)
  {
    logDebugInfo("Solution accepted. Current objective updated.");
    logDebugInfo("New accepted solution found. current objective updated to: " +
                 std::to_string(current_objective));
    logDebugInfo("New accepted solution found. _current_int_config updated to: " +
                 vectorToString(_current_int_config));
  }
}

Real
SimulatedAnnealingAlgorithm::acceptProbability(Real curr_obj, Real neigh_obj, Real curr_temp) const
{
  // Metropolis criterion
  Real delta_obj = neigh_obj - curr_obj;
  Real aprob;
  if (-delta_obj / curr_temp <= -700.0)
    aprob = 0.0;
  else if (-delta_obj / curr_temp >= 700.0)
    aprob = 1.0;
  else
    // aprob = std::exp(-delta_obj / curr_temp);
    aprob = std::exp(-delta_obj / curr_temp);

  if (delta_obj <= 0.0)
    aprob = 1.0;

  if (std::isnan(aprob))
    aprob = 0.0;
  return aprob;
}

Real
SimulatedAnnealingAlgorithm::coolingSchedule(unsigned int step) const
{

  auto fraction = (((Real)_max_its - (Real)step) / (Real)_max_its);

  switch (_cooling)
  {
    case LinAdd:
      return _temp_min + (_temp_max - _temp_min) * fraction;

    case ExpMult:
      return _temp_max * std::pow(_alpha, (Real)step);

    case QuadAdd:
      return _temp_min + (_temp_max - _temp_min) * std::pow(fraction, 2.0);

    case trial:
      return _temp_max * std::pow(_temp_min / _temp_max, (Real)step / (Real)_max_its);

    case LinMult:
      return _temp_max / (1.0 + _alpha * (Real)step);

    case LogMult:
      return _temp_max / (1.0 + _alpha * std::log10((Real)step + 1.0));

    case QuadMult:
      return _temp_max / (1.0 + _alpha * std::pow((Real)step, 2.0));

    case ExpAdd:
      return _temp_min + (_temp_max - _temp_min) /
                             (1.0 + std::exp(2.0 * std::log(_temp_max - _temp_min) *
                                             ((Real)step - 0.5 * (Real)_max_its) / (Real)_max_its));
    case TrigAdd:
      return _temp_min +
             0.5 * (_temp_max - _temp_min) * (1.0 + std::cos((Real)step * M_PI / (Real)_max_its));

    default:
      ::mooseError("Cooling option not yet implemented! Please choose from the follwoing options: "
                   "LinMult, ExpMult, LogMult, QuadMult, LinAdd, QuadAdd, ExpAdd, TrigAdd, trial");
  }

  return 1;
}

void
SimulatedAnnealingAlgorithm::updateBestSolution(
    const Real & current_objective,
    const std::vector<Real> & current_real_config_copy,
    const std::vector<int> & current_int_config_copy,
    std::vector<Real> & best_real_config,
    std::vector<int> & best_int_config,
    std::vector<std::pair<Real, std::vector<int>>> & solutions)
{
  _min_objective = current_objective;
  best_real_config = current_real_config_copy;
  // best_int_config = _current_int_config;
  best_int_config = current_int_config_copy;

  // If the solutions set isn't at the initial desired size, just add the new solution
  if (solutions.size() < 10)
  {
    solutions.push_back(std::make_pair(_min_objective, best_int_config));
  }

  // Each time you find a better solution, add it to the vector
  // solutions.push_back(std::make_pair(_min_objective, best_int_config));

  // Find the pair with the worst objective value using the iterators functionality in C++
  auto worst_it = std::max_element(solutions.begin(), solutions.end());

  // If the current objective is better than the worst stored one, replace it
  if (current_objective < worst_it->first)
  {
    // When we dereference worst_it and assign a value to it with the line:
    *worst_it = std::make_pair(_min_objective, best_int_config);
    // we are effectively updating the actual element in the solutions vector that the iterator
    // worst_it is pointing to.
  }

  if (_debug_on)
  {
    logDebugInfo("New best solution found. _min_objective updated to: " +
                 std::to_string(_min_objective));

    logDebugInfo("New best solution found. best_int_configuration updated to: " +
                 vectorToString(best_int_config));
  }
}

void
SimulatedAnnealingAlgorithm::coolAndReset(Real & temp_current,
                                          Real & current_objective,
                                          const std::vector<Real> & best_real_config,
                                          const std::vector<int> & best_int_config)
{
  // perform non-monotonic adjustment if applicable
  if (!_monotonic_cooling)
    temp_current *= (1.0 + (current_objective - _min_objective) / current_objective);

  // rewind to best value if reset is enabled
  if (std::abs(temp_current) <= _res_var)
  {
    _res_var *= 0.5;
    current_objective = _min_objective;
    _current_real_config = best_real_config;
    _current_int_config = best_int_config;
  }
}

void
SimulatedAnnealingAlgorithm::storeBestSolution(
    Real & current_objective,
    const std::vector<Real> & best_real_config,
    const std::vector<int> & best_int_config,
    std::vector<std::pair<Real, std::vector<int>>> & best_solutions,
    std::vector<std::pair<Real, std::vector<int>>> & solutions)
{
  // Select the best state we ended up finding
  current_objective = _min_objective;
  _current_real_config = best_real_config;
  _current_int_config = best_int_config;

  // Add the ultimate best solution to the vector
  solutions.push_back(std::make_pair(current_objective, _current_int_config));

  // Find the pair with the worst objective value using the iterators functionality in C++
  auto worst_it = std::max_element(solutions.begin(), solutions.end());

  // If the current objective is better than the worst stored one, replace it
  if (current_objective < worst_it->first)
  {
    // When we dereference worst_it and assign a value to it with the line:
    *worst_it = std::make_pair(current_objective, _current_int_config);
    // we are effectively updating the actual element in the solutions vector that the iterator
    // worst_it is pointing to.
  }

  // Sort the solutions vector
  std::sort(solutions.begin(), solutions.end());

  // Add the best solution to _best_solutions and sort
  best_solutions.push_back(solutions[0]);

  // After each run, sort the _best_solutions vector in ascending order of the objective function
  // value
  std::sort(best_solutions.begin(), best_solutions.end());
}
#include <filesystem>

void
SimulatedAnnealingAlgorithm::printBestSolutions(
    const std::vector<std::pair<Real, std::vector<int>>> & best_solutions)
{
  std::ofstream file;

  // Use std::filesystem to get the path in the current working directory
  std::filesystem::path file_path = std::filesystem::current_path() / "best_solutions.txt";

  // Open the file.
  file.open(file_path);

  // Check if the file was opened successfully
  if (!file.is_open())
  {
    mooseError("Failed to open the output file:", file_path.string());
  }

  std::cout << "The best solutions for all runs are as follows:" << std::endl;
  file << "The best solutions for all runs are as follows:" << std::endl;

  for (const auto & pair : best_solutions)
  {
    std::cout << "Objective value: " << std::fixed << std::setprecision(7) << pair.first
              << ", Solution: ";
    file << "Objective value: " << std::fixed << std::setprecision(7) << pair.first
         << ", Solution: ";
    for (const auto & val : pair.second)
    {
      std::cout << val << " ";
      file << val << " ";
    }
    std::cout << std::endl;
    file << std::endl;
  }

  std::pair<Real, std::vector<int>> best_pair = best_solutions[0];

  std::cout << "\nThe overall best solution is:\n";
  std::cout << "Objective value: " << std::fixed << std::setprecision(7) << best_pair.first
            << ", Solution: ";

  file << "\nThe overall best solution is:\n";
  file << "Objective value: " << std::fixed << std::setprecision(7) << best_pair.first
       << ", Solution: ";

  for (const auto & val : best_pair.second)
  {
    std::cout << val << " ";
    file << val << " ";
  }
  std::cout << std::endl;
  file << std::endl;
  file.flush(); // Flush before closing
  file.close(); // Close the file after writing
}

void
SimulatedAnnealingAlgorithm::logDebugInfo(const std::string & info)
{
  std::ofstream logfile;

  // Using std::filesystem to get the path in the current working directory
  std::filesystem::path cwd = std::filesystem::current_path() / "debug_log.txt";

  // Static variable to check if it's the first call to the function
  static bool firstCall = true;

  if (firstCall)
  {
    logfile.open(cwd, std::ios_base::out); // Overwrite or create new file
    firstCall = false;                     // Set flag to false so next calls will append
  }
  else
  {
    logfile.open(cwd, std::ios_base::app); // Appending to the file
  }

  if (logfile.is_open())
  {
    logfile << info << std::endl;
    logfile.close();
  }
  else
  {
    std::cerr << "Failed to open debug logfile!" << std::endl;
  }
}

std::string
SimulatedAnnealingAlgorithm::vectorToString(const std::vector<int> & vec)
{
  std::string result = "[";
  for (size_t i = 0; i < vec.size(); ++i)
  {
    result += std::to_string(vec[i]);
    if (i != vec.size() - 1)
    {
      result += " ";
    }
  }
  result += "]";
  return result;
}

std::string
SimulatedAnnealingAlgorithm::pairToString(const std::pair<Real, std::vector<int>> & p)
{
  return "Objective: " + std::to_string(p.first) + ", Configuration: " + vectorToString(p.second);
}
