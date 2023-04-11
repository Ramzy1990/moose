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

    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0))

{

  _initial_material_used =
      isParamValid("initial_material") ? getParam<subdomain_id_type>("initial_material") : 0;

  _assign_type = isParamValid("assign_type") ? getParam<std::string>("assign_type") : "manual";

  _mesh = _fe_problem.mesh();

  _total_cells =
      isParamValid("number_of_elements") ? getParam<dof_id_type>("number_of_elements") : 9;

  // _allowed_parameter_values = isParamValid("allowed_mateirals")
  //                                 ? getParam<std::vector<SubdomainName>>("allowed_mateirals")
  //                                 : {'f', 'm', 'v'};

  if (isParamValid("allowed_mateirals"))
    _allowed_parameter_values = getParam<std::vector<subdomain_id_type>>("allowed_mateirals");
  else
    _allowed_parameter_values = {0, 1};

  // _cell_subdomain_id = isParamValid("target_cell_subdomain_id")
  //                          ? getParam<std::vector<SubdomainName>>("target_cell_subdomain_id")
  //                          : {"f", "f", "f", "m", "m", "m", "f", "f", "f"};

  // _cell_pattern = isParamValid("cell_pattern") ?
  // getParam<std::vector<dof_id_type>>("cell_pattern")
  //                                              : {1, 2, 3, 4, 5, 6, 7, 8, 9};

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
  _initial_elements_to_optimize.resize(_total_cells);
  _initial_subdomains_to_optimize.resize(_total_cells);
  // _cell_subdomain_id.resize(_total_cells);

  //***********************
  // Calling The Functions
  //***********************

  // Checking if the materials used in the mesh/test are allowed or not.
  isMaterialAllowed();

  // initialization of the domain to be optimized. This will call a function to get the
  // current optimization domain and assign this domain to respective variables. Those could
  // be considered as the previous domain when it is not an initialization process.
  setInitialCondition();

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
    // updateSubdomainID(_allowed_parameter_values, _initial_cell_subdomain_id, _cell_subdomain_id);
  };

  //*******************
  // Print the Results
  //*******************
  // for (const auto & s : _cell_subdomain_id) // allowed starting from C++11
  //   std::cout << s << '\n';

  // Print the updated map
  for (const auto & pair : _pairs_to_optimize)
  {
    std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
  }
}

//***********************
// Functions Definitions
//***********************

// Checking if the materials used are allowed
void
DiscreteOptimizationReporter::isMaterialAllowed()
{

  // Check if initial_material_used is in allowed_parameter_values
  // By using a reference as the loop variable type, you avoid making a copy of each element
  // in the container. Instead, you get a reference to each element in the container, which is
  // faster and more efficient.
  bool is_allowed = false;

  // For manual insertion, for testing purposes, use general loop element.
  // we are checking for the inital material used to initialize the domain.
  if (_assign_type == "manual")
  {
    for (subdomain_id_type & element : _allowed_parameter_values)
    {
      if (element == _initial_material_used)
      {
        is_allowed = true;
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
    for (auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    {

      if (std::find(_allowed_parameter_values.begin(),
                    _allowed_parameter_values.end(),
                    elem->subdomain_id()) == _allowed_parameter_values.end())

        // Element subdomain_id is not in the vector
        mooseError("The initial material used for the domain is not allowed in this optimization! "
                   "Please check your input file");
    }
  }
}

void
DiscreteOptimizationReporter::getOptimizationDomain()
{

  // Loop over elements in the mesh
  for (auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    _elements_to_optimize.push_back(elem->id());
    _subdomains_to_optimize.push_back(elem->subdomain_id());
    _pairs_to_optimize.insert(
        std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
  }

  // elements are going to not change, but subdomains will be reassigned.
  // ChangeSubdomainAssignment::setSubdomainAssignment(
  //     const std::map<dof_id_type, SubdomainID> & assignment) const
  // for (auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  // {
  //   auto p = assignment.find(elem->id());
  //   if (p != assignment.end())
  //     elem->subdomain_id() = p->second;
  // }
  // _mesh.update();
}

// void
// DiscreteOptimizationReporter::setInitialCondition(
//     std::vector<dof_id_type> & _initial_elements_to_optimize,
//     std::vector<subdomain_id_type> & _initial_subdomains_to_optimize,
//     std::pair<dof_id_type, subdomain_id_type> & _pairs_to_optimize)

// Set initial subdomain ids from the initial material used, or from the domain's mesh.
void
DiscreteOptimizationReporter::setInitialCondition()
{

  // For manual insertion, for testing purposes, use general loop element.
  // Initialization is also done based on the data read and not the mesh.
  if (_assign_type == "manual")
  {
    // Loop to add elements to _initial_cell_subdomain_id given number of elements
    for (dof_id_type i = 0; i < _total_cells; i++)
    {
      // Seed the random number generator
      std::srand(std::time(0));
      _initial_elements_to_optimize.push_back(
          rand() % 1000); // for testing purposes, it does not matter what is the element id.
      _initial_subdomains_to_optimize.push_back(_initial_material_used);
      _initial_pairs_to_optimize.insert(std::pair<dof_id_type, subdomain_id_type>(
          _initial_elements_to_optimize[i], _initial_subdomains_to_optimize[i]));
      // _initial_cell_subdomain_id[i] = _initial_material_used;
    }
  }
  else
  {
    getOptimizationDomain(); // get the optimization domain mesh.

    // The initialization is done such that we have two vectors, one for the initial
    // (eventually the previous domain), and one for the current domain.
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
  if (pairs_to_optimize.size() != previous_pairs_to_optimize.size())
    mooseError("The sizes of the subdomainIDs do not match! Please make sure the previous "
               "subdomainIDs size is the same as the new one");

  // Setting the current Subdomain ID to the previous one.
  // note that the elements ID are the same.
  // This is true as long as "second" are of the same data type.
  if (typeid(pairs_to_optimize) == typeid(previous_pairs_to_optimize))
  {
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

  // Randomize the contents of cell_subdomain_id using "f" and "m" from _allowed_parameter_values
  // for (auto i = 0; i < pairs_to_optimize.size(); i++)
  // {
  //   // Generate a random number between 0 and 1 only (hence "f" or "m").
  //   int random_index = std::rand() % 2;
  //   pairs_to_optimize[i] =
  //       _allowed_parameter_values[random_index]; // Assign either "f" or "m" randomly
  // }

  //************************************************************************************************************************//

  // 2- Randomizing the current subdomain ID based on some criterion

  // Define the threshold for the area of a circle
  Real area_threshold = 7853.0;
  std::vector<subdomain_id_type> some_variable = allowed_parameter_values;
  // // // Randomize the contents of cell_subdomain_id based on the area of a circle
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

  //************************************************************************************************************************//

  // 3- Using Simulated Annealing (Kinda?!)

  // Function to calculate the cost of a given Domain
  // Used to update the total_cost of the domain.

  // Simulated annealing parameters
  // There could be other parameters as well
  // unsigned int iterations = 77;
  // Real cooling_rate = 0.77;

  // initializing the temperature before the loop
  // Real current_temperature = 77.0;

  // for (unsigned int i = 0; i < iterations; i++)
  //   {
  //     // Generate a random neighbor by changing a random element in cell_subdomain_id
  //     std::vector<subdomain_id_type> neighbor = cell_subdomain_id;
  //     unsigned int random_index = std::rand() % neighbor.size();

  //     // "f" and "m" are at index 0 and 1 in _allowed_parameter_values, hence taking the
  //     // reminder division by 2, 0 or 1.
  //     unsigned int random_value_index = std::rand() % 2;

  //     neighbor[random_index] = allowed_parameter_values[random_value_index];

  //     // After assigning the neighbor element, we test this new domain (where just one element has
  //     // changed).

  //     Real current_cost = costFunction(cell_subdomain_id);
  //     Real neighbor_cost = costFunction(neighbor);

  //     // Calculate the cost difference
  //     Real cost_difference = neighbor_cost - current_cost;

  //     // If the neighbor has a lower cost, accept it
  //     if (cost_difference < 0)
  //     {
  //       cell_subdomain_id = neighbor;
  //     }
  //     else
  //     {
  //       // Otherwise, accept the neighbor with a probability depending on the temperature
  //       Real acceptance_probability = std::exp(-cost_difference / current_temperature);
  //       if ((std::rand() / static_cast<double>(RAND_MAX)) < acceptance_probability)
  //       {
  //         cell_subdomain_id = neighbor;
  //       }
  //     }

  //     // Cool down the temperature
  //     current_temperature *= cooling_rate;
  //   }
}

// Compute the cost function given some domain.
Real
DiscreteOptimizationReporter::costFunction(const std::vector<subdomain_id_type> & Domain)
{
  // Define our cost function here based on the Domain
  // For example, we can count the number of consecutive "f"s or "m"s!
  unsigned int total_cost = 0;

  // Starting from 1 since the first element in domain starts at 0 = i - 1.
  for (subdomain_id_type i = 1; i < Domain.size(); i++)
  {
    if (Domain[i] == Domain[i - 1])
    {
      total_cost++;
    }
  }
  return total_cost;
}
