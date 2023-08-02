//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimulatedAnnealingAlgorithm.h"
#include <limits>

// The vector_hash struct
// Taken from: https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
// Used to hash the solutions cache with a map!
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

SimulatedAnnealingAlgorithm::SimulatedAnnealingAlgorithm()
  : CustomOptimizationAlgorithm(),
    _alpha(1e-2),
    _temp_max(100.0),
    _temp_min(0.01),
    _min_objective(std::numeric_limits<Real>::max()),
    _cooling(trial), /*    LinMult, ExpMult, LogMult, QuadMult, LinAdd, QuadAdd, ExpAdd, TrigAdd */
    _monotonic_cooling(false), /* Whether cooling is monotonic or not */
    _res_var(1), /*the initial temperature to start resetting the current state to the best found
                       state when this temperature is reached. This temperature is halved every-time
                       it is reached.*/
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
  // check that solution size has been set
  if (_size == 0)
    mooseError("Solution size is zero. Most likely setInitialSolution was not called.");

  // check neighbor generation options for int params
  if (_int_state_size > 0 && _num_reassignments > 0 && _valid_options.size() < 2)
    mooseError("If the number of reassignments for neigbor generation is > 0, then the number of "
               "valid options must be at least 2.");

  if (_int_state_size > 0 && _num_swaps + _num_reassignments == 0)
    mooseError("The problem has a non-zero number of categorical parameters, but the number of "
               "swaps and number of reassignments for neighbor generation are both 0.");

  int num_runs = 1000;                                           // number of runs to perform
  std::vector<std::pair<Real, std::vector<int>>> best_solutions; // Store best solutions of each

  // set initial neighbor & best states
  std::vector<Real> init_real_solution = _current_real_solution;
  std::vector<int> init_int_solution = _current_int_solution;
  std::vector<Real> init_best_real_solution = _current_real_solution;
  std::vector<int> init_best_int_solution = _current_int_solution;

  Real current_objective;
  objectiveFunction(current_objective, _current_real_solution, _current_int_solution, _ctx);

  Real initial_objective = current_objective;
  _min_objective = initial_objective; // Initialize the minimum objective

  std::vector<Real> neighbor_real_solution;
  std::vector<int> neighbor_int_solution;
  std::vector<Real> best_real_solution;
  std::vector<int> best_int_solution;

  for (int run = 0; run < num_runs; ++run)
  {
    // initialization code for each run...
    _it_run = run;
    // the number of "accepted" steps [not necessarily equal to # of traversals of while loop]
    _it_counter = 0;

    _res_var = 1.0;

    // Sets the seed for the random number generator
    MooseRandom::seed(std::time(0) + run);

    Real temp_current = _temp_max; // Current temperature is set to maximum one.

    // Tabu list
    const int max_tabu_size = 100; // maximum size of the Tabu list
    std::deque<std::vector<int>> tabu_list;

    // Solution cache
    std::unordered_map<std::vector<int>, Real, vector_hash> solution_cache;

    neighbor_real_solution = init_real_solution;
    best_real_solution = init_real_solution;

    // Initialize solutions
    if (run == 0)
    {
      neighbor_int_solution = init_int_solution;
      best_int_solution = init_int_solution;
    }
    else
    {
      neighbor_int_solution = best_solutions.front().second;
      best_int_solution = best_solutions.front().second;
      current_objective =
          best_solutions.front().first;   // Update the current objective with the objective value
                                          // from the best solution of the previous runs
      _min_objective = current_objective; // Minimum objective is e_best in the fortran code
    }

    // Keeping track of the accepted solutions!
    // This initializes with 3 pairs, each having a maximum possible objective value and an empty
    // vector of integers
    std::vector<std::pair<Real, std::vector<int>>> solutions(
        3, std::make_pair(std::numeric_limits<Real>::max(), std::vector<int>()));

    // simulated annealing loop
    while (_it_counter < _max_its && std::abs(temp_current) > _temp_min)
    {

      // Sets the seed for the random number generator
      // std::srand(std::time(0));
      // MooseRandom::seed(std::time(0) + _it_counter);

      // get a new neighbor and compute energy
      createNeigborReal(_current_real_solution, neighbor_real_solution);
      createNeigborInt(_current_int_solution, neighbor_int_solution, _execlude_domain);

      // Check if neighbor is in Tabu list
      _tabu_used = false;
      if (std::find(tabu_list.begin(), tabu_list.end(), neighbor_int_solution) != tabu_list.end())
      {
        // Neighbor is in Tabu list, skip this iteration
        _tabu_used = true;
        continue;
      }

      Real neigh_objective;

      _cache_used = false;
      auto it = solution_cache.find(neighbor_int_solution);
      if (it != solution_cache.end())
      {
        // If the solution is in the cache, use the cached objective value
        neigh_objective = it->second;
        _cache_used = true;
      }
      else
      {
        // Otherwise, compute the objective value and add it to the cache
        objectiveFunction(neigh_objective, neighbor_real_solution, neighbor_int_solution, _ctx);
        solution_cache[neighbor_int_solution] = neigh_objective;
      }

      // std::cout << "Here is the neighbour objective value:  " << neigh_objective << " !\n\n\n";
      // std::cout << "Here is the current objective value:  " << current_objective << " !\n\n\n";

      // Copy the current solution
      std::vector<int> current_int_solution_copy = _current_int_solution;

      // acceptance check: lower objective always accepted;
      // acceptance check: lower temps always accepted;
      // higher objective sometimes accepted
      _solution_accepted = false;
      Real temp_r = MooseRandom::rand();
      if (temp_r <= acceptProbability(current_objective, neigh_objective, temp_current))
      {
        // if we accept then it always counts as a new step
        ++_it_counter;
        _current_real_solution = neighbor_real_solution;
        _current_int_solution = neighbor_int_solution;
        current_objective = neigh_objective;
        _solution_accepted = true;
        // Add the current solution to the Tabu list
        tabu_list.push_back(_current_int_solution);

        // If the Tabu list is too big, remove the oldest element
        if (tabu_list.size() > max_tabu_size)
        {
          tabu_list.pop_front();
        }

        // std::cout << "IN THIS STEP, WE ACCEPTED THE SOLUTION!"
        //           << "\n\n\n";
        // std::cout << "The current Objective value is: " << current_objective << "!\n";
        // std::cout << "The neighbor Objective value is: " << neigh_objective << "!\n";
      }
      else
      {
        // otherwise, it has a 10% chance to count as a new step to finish the problem
        // this is especially important for combinatorial problems
        Real temp_rr = MooseRandom::rand();
        if (temp_rr <= 0.1)
          ++_it_counter;

        // std::cout << "IN THIS STEP, WE DID NOT ACCEPT!"
        // << "\n\n\n";
      }

      // cool the temperature
      temp_current = coolingSchedule(_it_counter);

      // if this is the best energy, it's our new best value
      if (current_objective < _min_objective)
      {
        _min_objective = current_objective;
        best_real_solution = _current_real_solution;
        // best_int_solution = _current_int_solution;
        best_int_solution = current_int_solution_copy;

        // // Each time you find a better solution, add it to the vector
        solutions.push_back(std::make_pair(_min_objective, best_int_solution));

        // // Find the pair with the worst objective value
        // // Here we are using the iterators functionality in C++
        auto worst_it = std::max_element(solutions.begin(), solutions.end());

        // // If the current objective is better than the worst stored one, replace it
        if (current_objective < worst_it->first)
        {
          *worst_it = std::make_pair(_min_objective, best_int_solution);
        }
      }

      // perform non-monotonic adjustment if applicable
      if (!_monotonic_cooling)
        temp_current *= (1.0 + (current_objective - _min_objective) / current_objective);

      // rewind to best value if reset is enabled
      if (std::abs(temp_current) <= _res_var)
      {
        _res_var *= 0.5;
        current_objective = _min_objective;
        _current_real_solution = best_real_solution;
        _current_int_solution = best_int_solution;
      }
    }

    // select the best state we ended up finding
    current_objective = _min_objective;
    _current_real_solution = best_real_solution;
    _current_int_solution = best_int_solution;

    // Add the ultimate best solution to the vector
    solutions.push_back(std::make_pair(current_objective, _current_int_solution));

    // // Find the pair with the worst objective value
    // // Here we are using the iterators functionality in C++
    auto worst_it = std::max_element(solutions.begin(), solutions.end());

    // // If the current objective is better than the worst stored one, replace it
    if (current_objective < worst_it->first)
    {
      *worst_it = std::make_pair(current_objective, _current_int_solution);
    }

    // Now sort the vector
    std::sort(solutions.begin(), solutions.end());

    std::pair<Real, std::vector<int>> best_pair_one_run = solutions[0];

    // best_solutions.push_back(std::make_pair(_min_objective, _current_int_solution));
    best_solutions.push_back(best_pair_one_run);
    // After each run, sort the best_solutions vector in ascending order of the objective function
    // value
    std::sort(best_solutions.begin(), best_solutions.end());

    // std::sort(all_accepted_solutions.begin(), all_accepted_solutions.end());

    // Insert the best 3 solutions into the solutions vector
    // for (std::size_t i = 0; i < 3 && i < all_accepted_solutions.size(); i++)
    // {
    //   solutions[i] = all_accepted_solutions[i];
    // }
  }

  // std::pair<Real, std::vector<int>> best_pair = solutions[0];
  std::pair<Real, std::vector<int>> best_pair = best_solutions[0];

  // // Print the best solution and its objective value
  // std::cout << "Best objective value: " << best_pair.first << "\n";
  // std::cout << "Associated solution: ";
  // for (int value : best_pair.second)
  // {
  //   std::cout << value << " ";
  // }
  // std::cout << "\n";

  std::cout << "The best solutions for all runs are as follows: \n";
  for (const auto & pair : best_solutions)
  {
    std::cout << "Objective value: " << pair.first << ", Solution: ";
    for (const auto & val : pair.second)
    {
      std::cout << val << " ";
    }
    std::cout << "\n";
  }

  std::cout << "The overall best solution is:\n";
  std::cout << "Objective value: " << best_pair.first << ", Solution: ";
  for (const auto & val : best_pair.second)
  {
    std::cout << val << " ";
  }
  std::cout << "\n";
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
      return _temp_max * std::pow(_alpha, step);

    case QuadAdd:
      return _temp_min + (_temp_max - _temp_min) * std::pow(fraction, 2);
    case trial:
      return _temp_max * std::pow(_temp_min / _temp_max, (Real)step / (Real)_max_its);

    default:
      ::mooseError("Cooling option not yet implemented! Please choose from the follwoing options: "
                   "LinMult, ExpMult, LogMult, QuadMult, LinAdd, QuadAdd, ExpAdd, TrigAdd, trial");
  }

  return 1;
}

void
SimulatedAnnealingAlgorithm::createNeigborReal(const std::vector<Real> & real_sol,
                                               std::vector<Real> & real_neigh) const
{
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

void
SimulatedAnnealingAlgorithm::createNeigborInt(const std::vector<int> & int_sol,
                                              std::vector<int> & int_neigh,
                                              const std::vector<int> & exclude_values) const
{
  // One and two values
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

  ///////////////////////////////////////////////////////////
  //// Many vlaues but flipping just one element at a time///
  ///////////////////////////////////////////////////////////
  if (_int_size == 0)
  {
    int_neigh = {};
    return;
  }

  unsigned int index;
  int_neigh = int_sol;
  // Get the min and max values from int_sol
  int min_value = *std::min_element(int_sol.begin(), int_sol.end());
  int max_value = *std::max_element(int_sol.begin(), int_sol.end());

  // Find a random index whose value is not in exclude_values
  do
  {
    index = MooseRandom::randl() % _int_size;
  } while (std::find(exclude_values.begin(), exclude_values.end(), int_neigh[index]) !=
           exclude_values.end());

  // Generate a new random value different from the current one and not in exclude_values
  int new_val;
  do
  {
    // Adjust the modulus operation to be the size of our desired range (max_value - min_value + 1)
    // Add the minimum value, min_value, to shift this range up from starting at 0 to starting at
    // our minimum value.
    new_val = MooseRandom::randl() % (max_value - min_value + 1) + min_value;
  } while (new_val == int_neigh[index] ||
           std::find(exclude_values.begin(), exclude_values.end(), new_val) !=
               exclude_values.end());

  int_neigh[index] = new_val;

  // Check if all possible values are in the new solution
  std::vector<bool> hasValue(max_value - min_value + 1, false);
  for (unsigned int i = 0; i < _int_size; ++i)
  {
    hasValue[int_neigh[i] - min_value] = true;
  }

  // If any value is missing (and not in exclude_values), insert it at a random position
  for (int i = 0; i <= max_value - min_value; ++i)
  {
    if (!hasValue[i] && std::find(exclude_values.begin(), exclude_values.end(), i + min_value) ==
                            exclude_values.end())
    {
      int random_index;
      // Find a random index whose value is not in exclude_values
      do
      {
        random_index = MooseRandom::randl() % _int_size;
      } while (std::find(exclude_values.begin(), exclude_values.end(), int_neigh[random_index]) !=
               exclude_values.end());

      int_neigh[random_index] = i + min_value;
      // Assuming the distribution is uniform, we do not need to re-check after inserting
      break;
    }
  }

  ///////////////////////////////////////////////////////////
  ///////// Many vlaues but flipping a lot at a time ////////
  ///////////////////////////////////////////////////////////
  // if (_int_size == 0)
  // {
  //   int_neigh = {};
  //   return;
  // }

  // Real flip_ratio = 0.50; // Adjust this to the desired flip ratio
  // unsigned int num_flips = static_cast<unsigned int>(_int_size * flip_ratio);
  // if (num_flips > _int_size)
  // {
  //   num_flips = _int_size; // ensure we do not try to flip more elements than we have
  // }

  // // Get the min and max values from int_sol
  // subdomain_id_type min_value = *std::min_element(int_sol.begin(), int_sol.end());
  // subdomain_id_type max_value = *std::max_element(int_sol.begin(), int_sol.end());
  // int_neigh = int_sol; // copy the solution into the neighbor

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

void
SimulatedAnnealingAlgorithm::setValidReassignmentOptions(const std::set<int> & options)
{
  if (options.size() == 0)
    mooseError("Empty set of options");
  _valid_options.clear();
  _valid_options.resize(options.size());
  unsigned int j = 0;
  for (auto & p : options)
    _valid_options[j++] = p;
}

void
SimulatedAnnealingAlgorithm::setLowerLimits(const std::vector<Real> & lower_limits)
{
  _lower_limit_provided = true;
  _parameter_lower_limit = lower_limits;
  if (_parameter_lower_limit.size() != _real_size)
    mooseError("Lower limits has incorrect size");
}

void
SimulatedAnnealingAlgorithm::setUpperLimits(const std::vector<Real> & upper_limits)
{
  _upper_limit_provided = true;
  _parameter_upper_limit = upper_limits;
  if (_parameter_upper_limit.size() != _real_size)
    mooseError("Upper limits has incorrect size");
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
