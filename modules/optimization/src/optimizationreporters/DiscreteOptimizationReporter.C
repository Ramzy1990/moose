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
#include "DiscreteOptimizationReporter.h"
// #include "FEProblem.h"
// #include "MooseMesh.h"

//*********************
// Regiester the MOOSE
//*********************
registerMooseObject("OptimizationApp", DiscreteOptimizationReporter);

//*************************
// Adding Input Parameters
//*************************
InputParameters
DiscreteOptimizationReporter::validParams()
{
  InputParameters params = OptimizationReporter::validParams();

  params.addClassDescription("Receives computed objective function, and contains reporters for "
                             "communicating between discreteOptimizeSolve and subapps.");

  params.addParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");

  params.addParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");

  params.addParam<subdomain_id_type>(
      "initial_material",
      "The initial material we are going to assign to all the cells in the domain.");

  params.addParam<std::string>("assign_type",
                               "String to decide the assign type for the cells pattern. Manual "
                               "where we assign manually the cells pattern. Automatic where "
                               "we assign using the mesh.");

  params.addParam<std::string>("solver_type",
                               "String to decide the solver type for the cells pattern. Random "
                               "where we assign randomly the new cells pattern. Random contains "
                               "thhree flavors. Options are random_1, random_2, and random_3.");

  params.addParam<dof_id_type>(
      "number_of_elements",
      "Number of elements or the size of the domain vector inside the domain");

  /// Allowed values for my cell_subdomain_ID (e.g., materials possible to use {f,m,v}).
  params.addParam<std::vector<subdomain_id_type>>(
      "allowed_mateirals",
      "Allowed values for the subdomain_ID (e.g., materials possible to use {0,1,2}).");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteOptimizationReporter::DiscreteOptimizationReporter(const InputParameters & parameters)
  : OptimizationReporter(parameters),

    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),

    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),

    _nparam(_parameter_names.size()),

    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0)),

    // _mesh_moose(_fe_problem.mesh()),

    // _mesh(_mesh_moose.getMesh()) // libmesh

    _mesh(_fe_problem.mesh())
{

  _initial_material_used =
      isParamValid("initial_material") ? getParam<subdomain_id_type>("initial_material") : 0;

  _assign_type = isParamValid("assign_type") ? getParam<std::string>("assign_type") : "manual";

  _solve_type = isParamValid("solver_type") ? getParam<std::string>("solver_type") : "random_1";

  _total_cells =
      isParamValid("number_of_elements") ? getParam<dof_id_type>("number_of_elements") : 9;

  if (isParamValid("allowed_mateirals"))
    _allowed_parameter_values = getParam<std::vector<subdomain_id_type>>("allowed_mateirals");
  else
    _allowed_parameter_values = {0, 1};

  //********************
  // Checking Data Read
  //********************
  // Checking the number of parameters to be obtimized in each category.
  // if (_parameter_names.size() != _nvalues.size())
  //   paramError("num_parameters",
  //              "There should be a number in 'num_parameters' for each name in
  //              'parameter_names'.");

  // Reserving and resizing the vectors, such that we can use them with loops iterators.
  // _initial_cell_subdomain_id.resize(_total_cells);
  // _initial_elements_to_optimize.resize(_total_cells);
  // _initial_subdomains_to_optimize.resize(_total_cells);
  // _cell_subdomain_id.resize(_total_cells);

  //***********************
  // Calling The Functions
  //***********************

  if (!_fe_problem.hasMultiApps())
  {
    std::cout << std::endl;
    std::cout << "The discrete optimization reported is running on the main app since it could not "
                 "find any sub apps!";
    std::cout << std::endl;

    // Checking if the materials used in the mesh/test are allowed or not.
    isMaterialAllowed(_mesh);

    // initialization of the domain to be optimized. This will call a function to get the
    // current optimization domain and assign this domain to respective variables. Those could
    // be considered as the previous domain when it is not an initialization process.
    setInitialCondition(_mesh);

    // Updating the optimization domain according to some logic.
    // This logic could be random (for tests), or it could be based on an optimizer (e.g.,
    // simulated annealing).
    if (_assign_type == "manual")
    {
      testUpdateSubdomainID(
          _allowed_parameter_values, _initial_pairs_to_optimize, _pairs_to_optimize);
    }
    else
    {
      updateSubdomainID(_allowed_parameter_values, _initial_pairs_to_optimize, _pairs_to_optimize);
    };

    //*******************
    // Print the Results
    //*******************
    // for (const auto & s : _cell_subdomain_id) // allowed starting from C++11
    //   std::cout << s << '\n';

    std::cout << std::endl;
    std::cout << "The old map is given as..." << std::endl;
    for (const auto & pair : _initial_pairs_to_optimize)
    {
      std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // Print the updated map
    std::cout << "The new updated map is..." << std::endl;
    for (const auto & pair : _pairs_to_optimize)
    {
      std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;
  } // End if (!_fe_problem.hasMultiApps())
}

//***********************
// Functions Definitions
//***********************

// Checking if the materials used are allowed
void
DiscreteOptimizationReporter::isMaterialAllowed(const MooseMesh & domain_mesh)
{

  // Check if initial_material_used is in allowed_parameter_values

  // By using references as the loop variable type, you avoid making a copy of each element
  // in the container. Instead, you get a reference to each element in the container, which is
  // faster and more efficient.
  bool is_allowed = false;

  // For manual insertion, for testing purposes, use general loop element.
  // we are checking for the inital material used to initialize the domain.
  // Furthermore, we are going to make the following work only if there are no subapps
  if (!_fe_problem.hasMultiApps())
  {
    if (_assign_type == "manual")
    {

      std::cout << std::endl;
      std::cout << std::endl;
      std::cout << "***Discrete Optimization Class: Manual MainApp Test Mode***" << std::endl;
      for (subdomain_id_type & element : _allowed_parameter_values)
      {
        if (element == _initial_material_used)
        {
          is_allowed = true;
          std::cout << std::endl;
          std::cout
              << "The Material used in the test is allowed according to the allowed_materials!"
              << std::endl;
          break;
        }
      }
      if (!is_allowed)
      {
        // Element is not in the vector
        mooseError("The initial material used for the domain is not allowed in this optimization! "
                   "Please check your input file");
      }
    }
    else // For automatic insertion using the generated mesh.
    {
      std::cout << std::endl;
      std::cout << "***Discrete Optimization Class: Automated MainApp Test Mode***" << std::endl;

      for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
      {

        if (std::find(_allowed_parameter_values.begin(),
                      _allowed_parameter_values.end(),
                      elem->subdomain_id()) == _allowed_parameter_values.end())
        {
          // Element subdomain_id is not in the vector
          mooseError(
              "The initial material used for the domain is not allowed in this optimization! "
              "Please check your input file");
        } // (std::find(_allowed_parameter_values.begin(), _allowed_parameter_values.end(),
          // elem->subdomain_id()) == _allowed_parameter_values.end())
      }   // for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
    }     // For automatic insertion using the generated mesh, and for no subapps.
  }       // (!_fe_problem.hasMultiApps())

  if (_fe_problem.hasMultiApps())
  {
    std::cout << std::endl;
    std::cout << "***Discrete Optimization Class: SubApp Mode***" << std::endl;

    // This could be better if it is checking for materials names.

    // elements owned by the processor (processor tag on it): active_local_element_ptr_range()
    for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
    {

      if (std::find(_allowed_parameter_values.begin(),
                    _allowed_parameter_values.end(),
                    elem->subdomain_id()) == _allowed_parameter_values.end())
      {
        // Element subdomain_id is not in the vector
        mooseError("The initial material used for the domain is not allowed in this optimization! "
                   "Please check your input file");
      }
    }
    std::cout << std::endl;
    std::cout << "Verification done successfully!... " << std::endl;
  }
}
void
DiscreteOptimizationReporter::execute()
{
  std::cout << std::endl;
  std::cout << "Executing the Discrete Optimization Reporter... " << std::endl;
  std::cout << std::endl;
  std::cout << "Now transfering TO the MultiAPP... " << std::endl;
  std::cout << "Well, basically, the TO Multiapp will *get* the updated mesh domain!... "
            << std::endl;
}

// bool
// DiscreteOptimizationReporter::isNewMaterialsInMesh(const MooseMesh & domain_mesh)
// {

//   // FIX ME: Add a check for the mesh changing

//   for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
//   {
//     unsigned int material_id = elem->subdomain_id();

//     // if (known_materials.find(material_id) == known_materials.end())
//     // {
//     // return true; // New material found
//     // }
//   }

//   return false; // No new materials found
// }

// Function to get the optimization domain of the problem using the elements of the mesh
void
DiscreteOptimizationReporter::getOptimizationDomain(const MooseMesh & domain_mesh)
{

  std::cout << "Assigning the elements and subdomains ids to their respective map from the mesh..."
            << std::endl;
  std::cout << std::endl;
  std::cout << "***Elements ids are your key, while subdomain ids are the respective values***"
            << std::endl;
  std::cout << std::endl;

  // Loop over elements in the mesh
  if (!_fe_problem.hasMultiApps())
  {
    for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
    // for (auto & elem : _mesh.getMesh().elem_ptr())
    {
      _elements_to_optimize.push_back(elem->id());
      _subdomains_to_optimize.push_back(elem->subdomain_id());
      _pairs_to_optimize.insert(
          std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
    }
    std::cout << std::endl;
    std::cout << "Assignment done successfully!" << std::endl;
    std::cout << std::endl;
  }

  // Loop over elements in the subapp mesh and get the domain
  if (_fe_problem.hasMultiApps())
  {
    for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
    // for (auto & elem : _mesh.getMesh().elem_ptr())
    {
      _elements_to_optimize.push_back(elem->id());
      _subdomains_to_optimize.push_back(elem->subdomain_id());
      _pairs_to_optimize.insert(
          std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
    }
    std::cout << std::endl;
    std::cout << "Assignment done successfully!" << std::endl;
    std::cout << std::endl;
  }
}

// Set initial subdomain ids from the initial material used, or from the domain's mesh
void
DiscreteOptimizationReporter::setInitialCondition(const MooseMesh & domain_mesh)
{

  if (!_fe_problem.hasMultiApps())
  {

    std::cout << std::endl;
    std::cout << "Initializing the domain..." << std::endl;
    std::cout << std::endl;

    // For manual insertion, for testing purposes, use general loop element.
    // Initialization is also done based on the data read and not the current mesh.
    if (_assign_type == "manual")
    {
      std::cout << std::endl;
      std::cout << "Manually initializing the domain using the test file information and random "
                   "numbers..."
                << std::endl;
      std::cout << std::endl;

      // Loop to add elements to our map vector given the number of elements
      for (dof_id_type i = 0; i < _total_cells; ++i)
      {

        // for testing purposes, it does not matter what is the element id.
        _initial_elements_to_optimize.push_back(i);

        _initial_subdomains_to_optimize.push_back(_initial_material_used);

        _initial_pairs_to_optimize.insert(std::pair<dof_id_type, subdomain_id_type>(
            _initial_elements_to_optimize.back(), _initial_subdomains_to_optimize.back()));

        _pairs_to_optimize.insert(std::pair<dof_id_type, subdomain_id_type>(
            _initial_elements_to_optimize.back(), _initial_subdomains_to_optimize.back()));
      }
    }
    else
    {
      std::cout << std::endl;
      std::cout << "Initializing the domain using the supplied mesh..." << std::endl;
      std::cout << std::endl;

      getOptimizationDomain(domain_mesh); // get the current optimization domain mesh.

      // The initialization is done such that we have two vectors, one for the initial
      // (eventually the previous domain), and one for the current domain.
      _initial_elements_to_optimize = _elements_to_optimize;
      _initial_subdomains_to_optimize = _subdomains_to_optimize;
      _initial_pairs_to_optimize = _pairs_to_optimize;
    }
  } // if (!_fe_problem.hasMultiApps())

  if (_fe_problem.hasMultiApps())
  {
    std::cout << std::endl;
    std::cout << "*** Discrete Otpimization Reporter: SubApp supplied mesh initialization..."
              << std::endl;
    std::cout << std::endl;
    getOptimizationDomain(domain_mesh); // get the current optimization domain mesh.

    // The initialization is done such that we have two vectors, one for the initial
    // (eventually the previous domain), and one for the current domain.
    // Hence maybe we can call the initilaization as the previousization! :)
    _initial_elements_to_optimize = _elements_to_optimize;
    _initial_subdomains_to_optimize = _subdomains_to_optimize;
    _initial_pairs_to_optimize = _pairs_to_optimize;
  }
}

// Set cell subdomainIDs to a new one taking into account the previous subdomainIDs in
// the domain.
void
DiscreteOptimizationReporter::testUpdateSubdomainID(
    const std::vector<subdomain_id_type> allowed_parameter_values,
    const std::map<dof_id_type, subdomain_id_type> previous_pairs_to_optimize,
    std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize)
{
  std::cout << std::endl;
  std::cout << "Updating the domain subdomain ids according to some random parameters..."
            << std::endl;
  std::cout << std::endl;

  std::cout << "***NOTE*** This is for testing purposes only!" << std::endl;
  std::cout << std::endl;

  if (pairs_to_optimize.size() != previous_pairs_to_optimize.size())
    mooseError("The sizes of the subdomainIDs do not match! Please make sure the previous "
               "subdomainIDs size is the same as the new one");

  // Setting the current Subdomain ID to the previous one.
  // note that the elements ID are the same.
  // This is true as long as "second" are of the same data type.
  if (typeid(pairs_to_optimize) == typeid(previous_pairs_to_optimize))
  {

    std::cout << "previous and current maps are of the same type! Proceeding to updating the "
                 "current domain map..."
              << std::endl;
    std::cout << std::endl;

    // Copy only the values from previous pairs to current pairs
    for (const auto & pair : previous_pairs_to_optimize)
    {
      if (pairs_to_optimize.find(pair.first) != pairs_to_optimize.end())
      {
        // Then the key exists in the current pair
        pairs_to_optimize[pair.first] = pair.second;
      }
    }
  }

  // Seed the random number generator for the randomization process in this test function.
  std::srand(std::time(0));

  //************************************************************************************************************************//

  // 1- Randomizing the current subdomain ID to a new one

  // Randomize the contents of pairs_to_optimize using "0" and "1" from _allowed_parameter_values
  if (_solve_type == "random_1")
  {

    for (auto it = pairs_to_optimize.begin(); it != pairs_to_optimize.end(); ++it)
    {
      // Generate a random number between 0 and 1 only (hence "f" or "m").
      int random_index = std::rand() % 2;
      it->second = _allowed_parameter_values[random_index]; // Assign randomly
    }
  }

  //************************************************************************************************************************//
  // 2- Randomizing the current subdomain ID based on some criterion

  else if (_solve_type == "random_2")
  {

    // Define the threshold for the area of a circle
    Real area_threshold = 7853.0;
    std::vector<subdomain_id_type> some_variable = allowed_parameter_values;
    // Randomize the contents of based on the area of a circle
    for (auto it = pairs_to_optimize.begin(); it != pairs_to_optimize.end(); ++it)
    {
      Real random_radius = std::rand() % 100; // Generate a random number between 0 and 99
      Real area = 3.141592 * random_radius * random_radius;

      if (area > area_threshold)
      {
        it->second = 0;
      }
      else
      {
        it->second = 1;
      }
    }
  }

  //************************************************************************************************************************//
  // 3- Using Simulated Annealing

  else if (_solve_type == "random_3")
  {

    // Simulated annealing parameters
    // There could be other parameters as well
    unsigned int iterations = 777;
    Real cooling_rate = 0.77;

    // initializing the temperature before the loop
    Real current_temperature = 77.0;

    for (unsigned int i = 0; i < iterations; i++)
    {

      // Generate a random neighbor by changing a random element in pairs_to_optimize
      std::map<dof_id_type, subdomain_id_type> neighbor = pairs_to_optimize;
      unsigned int random_index = std::rand() % neighbor.size();

      // "f" and "m" are at index 0 and 1 in _allowed_parameter_values, hence taking the
      // reminder division by 2, 0 or 1.
      unsigned int random_value_index = std::rand() % 2;

      // Advance the iterator to the random index
      auto random_iterator = neighbor.begin();
      std::advance(random_iterator, random_index);

      // Update the value at the random index
      random_iterator->second = allowed_parameter_values[random_value_index];

      // After assigning the element, we test the domain (where just one element has changed).
      unsigned int current_cost = costFunction(_mesh, pairs_to_optimize);
      unsigned int neighbor_cost = costFunction(_mesh, neighbor);

      // Calculate the cost difference
      int cost_difference = neighbor_cost - current_cost;

      // If the neighbor has a lower cost, accept it
      if (cost_difference < 0)
      {
        pairs_to_optimize = neighbor;
      }
      else
      {
        // Otherwise, accept the neighbor with a probability depending on the
        // temperature
        Real acceptance_probability = std::exp(-cost_difference / current_temperature);
        if ((std::rand() / static_cast<double>(RAND_MAX)) < acceptance_probability)
        {
          pairs_to_optimize = neighbor;
        }
      }

      // Cool down the temperature
      current_temperature *= cooling_rate;
    }
  }
}

// Set cell subdomainIDs to a new one taking into account the previous subdomainIDs
void
DiscreteOptimizationReporter::updateSubdomainID(
    const std::vector<subdomain_id_type> allowed_parameter_values,
    const std::map<dof_id_type, subdomain_id_type> previous_pairs_to_optimize,
    std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize)
{
  std::cout << std::endl;
  std::cout << "Updating the domain subdomain ids according to the supplied mesh..." << std::endl;
  std::cout << std::endl;

  if (pairs_to_optimize.size() != previous_pairs_to_optimize.size())
    mooseError("The sizes of the subdomainIDs do not match! Please make sure the previous "
               "subdomainIDs size is the same as the new one");

  if (!_fe_problem.hasMultiApps())
  {
    // Setting the current Subdomain ID to the previous one.
    // note that the elements ID are the same.
    // This is true as long as "second" are of the same data type.
    if (typeid(pairs_to_optimize) == typeid(previous_pairs_to_optimize))
    {

      std::cout << "previous and current maps are of the same type! Proceeding to updating the "
                   "current domain map..."
                << std::endl;
      std::cout << std::endl;

      // Copy only the values from previous pairs to current pairs
      for (const auto & pair : previous_pairs_to_optimize)
      {
        if (pairs_to_optimize.find(pair.first) != pairs_to_optimize.end())
        {
          // Then the key exists in the current pair
          pairs_to_optimize[pair.first] = pair.second;
        }
      }
    }

    // Seed the random number generator for the randomization process in this test function.
    std::srand(std::time(0));

    //************************************************************************************************************************//
    // Simulated annealing parameters
    // There could be other parameters as well
    unsigned int iterations = 777;
    Real cooling_rate = 0.77;

    // initializing the temperature before the loop
    Real current_temperature = 77.0;

    for (unsigned int i = 0; i < iterations; i++)
    {

      // Generate a random neighbor by changing a random element in pairs_to_optimize
      std::map<dof_id_type, subdomain_id_type> neighbor = pairs_to_optimize;
      unsigned int random_index = std::rand() % neighbor.size();

      // "f" and "m" are at index 0 and 1 in _allowed_parameter_values, hence taking the
      // reminder division by 2, 0 or 1.
      unsigned int random_value_index = std::rand() % 2;

      // Advance the iterator to the random index
      auto random_iterator = neighbor.begin();
      std::advance(random_iterator, random_index);

      // Update the value at the random index
      random_iterator->second = allowed_parameter_values[random_value_index];

      // After assigning the element, we test the domain (where just one element has changed).
      unsigned int current_cost = costFunction(_mesh, pairs_to_optimize);
      unsigned int neighbor_cost = costFunction(_mesh, neighbor);

      // Calculate the cost difference
      int cost_difference = neighbor_cost - current_cost;

      // If the neighbor has a lower cost, accept it
      if (cost_difference < 0)
      {
        pairs_to_optimize = neighbor;
      }
      else
      {
        // Otherwise, accept the neighbor with a probability depending on the temperature
        Real acceptance_probability = std::exp(-cost_difference / current_temperature);
        if ((std::rand() / static_cast<double>(RAND_MAX)) < acceptance_probability)
        {
          pairs_to_optimize = neighbor;
        }
      }

      // Cool down the temperature
      current_temperature *= cooling_rate;
    }
  } // if (!_fe_problem.hasMultiApps())

  if (_fe_problem.hasMultiApps())
  {

    // We can eventually separate the methods into ones for testing and ones for release.
  }
}

// Compute the cost function given some domain.
// Function to calculate the cost of a given Domain
unsigned int
DiscreteOptimizationReporter::costFunctionTest(
    const MooseMesh & domain_mesh, const std::map<dof_id_type, subdomain_id_type> & domain_map)
{
  unsigned int total_cost = 0;

  // To not count many times the neighbors to the same element
  std::set<std::pair<dof_id_type, dof_id_type>> visited_pairs;

  for (const auto & elem : domain_mesh.getMesh().active_element_ptr_range())
  {
    // Get the subdomain_id_type of the current element from the domain_map
    subdomain_id_type elem_subdomain_id = domain_map.at(elem->id());

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      if (neighbor != nullptr) // If not empty space beside the current element
      {
        // Get the subdomain_id_type of the neighbor element from the domain_map
        subdomain_id_type neighbor_subdomain_id = domain_map.at(neighbor->id());

        // Create a pair representing the current and neighboring elements, sorted by
        // their IDs
        std::pair<dof_id_type, dof_id_type> elem_pair = std::minmax(elem->id(), neighbor->id());

        // Only increment the cost if the pair has not been visited and the subdomain
        // IDs are not equal!
        if (visited_pairs.find(elem_pair) == visited_pairs.end() &&
            elem_subdomain_id == neighbor_subdomain_id)
        {
          total_cost++;
          visited_pairs.insert(elem_pair);
        }
      }
    }
  }

  // Return the total cost
  return total_cost;

  // // Iterate through the map, using the iterator to access the key-value pairs
  // auto it = domain_map.begin();
  // auto prev = it;

  // // Move the iterator to the second element, if there is one
  // if (it != domain_map.end())
  //   ++it;

  // // Iterate through the map starting from the second element
  // for (; it != domain_map.end(); ++it)
  // {
  //   // Compare the value (second) of the current pair with the value of the previous pair
  //   if (it->second == prev->second)
  //   {
  //     total_cost++;
  //   }
  //   // Update the prev iterator to point to the current iterator
  //   prev = it;
  // }

  // return total_cost;
}

// Compute the cost function given some domain.
// Function to calculate the cost of a given Domain
unsigned int
DiscreteOptimizationReporter::costFunction(
    const MooseMesh & domain_mesh, const std::map<dof_id_type, subdomain_id_type> & domain_map)
{
  unsigned int total_cost = 0;

  // To not count many times the neighbors to the same element
  std::set<std::pair<dof_id_type, dof_id_type>> visited_pairs;

  for (const auto & elem : domain_mesh.getMesh().active_element_ptr_range())
  {
    // Get the subdomain_id_type of the current element from the domain_map
    subdomain_id_type elem_subdomain_id = domain_map.at(elem->id());

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      if (neighbor != nullptr) // If not empty space beside the current element
      {
        // Get the subdomain_id_type of the neighbor element from the domain_map
        subdomain_id_type neighbor_subdomain_id = domain_map.at(neighbor->id());

        // Create a pair representing the current and neighboring elements, sorted by
        // their IDs
        std::pair<dof_id_type, dof_id_type> elem_pair = std::minmax(elem->id(), neighbor->id());

        // Only increment the cost if the pair has not been visited and the subdomain
        // IDs are not equal!
        if (visited_pairs.find(elem_pair) == visited_pairs.end() &&
            elem_subdomain_id == neighbor_subdomain_id)
        {
          total_cost++;
          visited_pairs.insert(elem_pair);
        }
      }
    }
  }

  // Return the total cost
  return total_cost;
}

void
DiscreteOptimizationReporter::printCurrentDomain(const dof_id_type & iteration)
{

  if (iteration == 1)
  {
    std::ofstream file_Initial;

    // Open the file
    file_Initial.open("Initial_Domain.txt");

    // Print the updated map
    file_Initial << "The Initial map is..." << std::endl;
    for (const auto & pair : _initial_pairs_to_optimize)
    {
      file_Initial << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }

    // close the file
    file_Initial.close();
  }
  else if (iteration > 1)
  {
    std::ofstream file;

    // Open the file
    file.open("Current_Domain.txt");

    // Print the updated map
    file << "The Current map is..." << std::endl;
    for (const auto & pair : _pairs_to_optimize)
    {
      file << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }

    // close the file
    file.close();
  }
}

void
DiscreteOptimizationReporter::setDomainPostProcessInformation(const dof_id_type & iteration)
{

  if (iteration == 1)
  {
  }
  else if (iteration > 1)
  {
  }

  return;
}

std::tuple<std::vector<subdomain_id_type> &,
           std::map<dof_id_type, subdomain_id_type> &,
           std::map<dof_id_type, subdomain_id_type> &>
DiscreteOptimizationReporter::getMeshParameters()
{
  return std::tie(_allowed_parameter_values, _initial_pairs_to_optimize, _pairs_to_optimize);
}
