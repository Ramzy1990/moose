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

/// Has been used in the optimization app source files.
/// Seems to allow the "index_range" method found in the following file.
#include "libmesh/int_range.h"

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
  // InputParameters params = GeneralReporter::validParams();
  InputParameters params = OptimizationReporter::validParams();
  // params += OptimizationReporter::validParams();

  params.addClassDescription("Receives computed objective function, and contains reporters for "
                             "communicating between discreteOptimizeSolve and subapps.");

  params.addParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");

  params.addParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");

  params.addParam<SubdomainName>(
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
  params.addParam<std::vector<SubdomainName>>(
      "allowed_mateirals",
      "Allowed values for my region_ID (e.g., materials possible to use {f,m,v}).");

  /// Assign the subdomain ID type that we would like to reach (for test purposes and learning).
  // params.addParam<std::vector<SubdomainName>>("target_cell_subdomain_id",
  //                                             "The subdomain ID type that is required.");

  /// Assign the cell pattern id that is going to be optimized.
  // params.addParam<std::vector<std::vector<dof_id_type>>>(
  //     "cell_pattern", "The cell pattern (cell ids) that is going to be optimized.");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteOptimizationReporter::DiscreteOptimizationReporter(const InputParameters & parameters)
  // : GeneralReporter(parameters),
  : OptimizationReporter(parameters),
    // Getting our data from the "".i" file
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _nparam(_parameter_names.size()),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0))
// _initial_material_used(getParam<SubdomainName>("initial_material")),
// _assign_type(getParam<std::string>("assign_type")),
// _total_cells(getParam<dof_id_type>("number_of_elements")),
// _allowed_parameter_values(getParam<std::vector<SubdomainName>>("allowed_mateirals"))
// _cell_subdomain_id(getParam<std::vector<SubdomainName>>("target_cell_subdomain_id")),

{

  _initial_material_used =
      isParamValid("initial_material") ? getParam<SubdomainName>("initial_material") : "f";

  _assign_type = isParamValid("assign_type") ? getParam<std::string>("assign_type") : "manual";

  _total_cells =
      isParamValid("number_of_elements") ? getParam<dof_id_type>("number_of_elements") : 9;

  // _allowed_parameter_values = isParamValid("allowed_mateirals")
  //                                 ? getParam<std::vector<SubdomainName>>("allowed_mateirals")
  //                                 : {'f', 'm', 'v'};

  if (isParamValid("allowed_mateirals"))
    _allowed_parameter_values = getParam<std::vector<SubdomainName>>("allowed_mateirals");
  else
    _allowed_parameter_values = {"f", "m", "v"};

  // _cell_subdomain_id = isParamValid("target_cell_subdomain_id")
  //                          ? getParam<std::vector<SubdomainName>>("target_cell_subdomain_id")
  //                          : {"f", "f", "f", "m", "m", "m", "f", "f", "f"};

  // _cell_pattern = isParamValid("cell_pattern") ?
  // getParam<std::vector<dof_id_type>>("cell_pattern")
  //                                              : {1, 2, 3, 4, 5, 6, 7, 8, 9};

  //********************
  // Checking Data Read
  //********************
  if (_parameter_names.size() != _nvalues.size())
    paramError("num_parameters",
               "There should be a number in 'num_parameters' for each name in 'parameter_names'.");

  // Reserving and resizing the vectors
  _initial_cell_subdomain_id.resize(_total_cells);
  _cell_subdomain_id.resize(_total_cells);

  //***********************
  // Calling The Functions
  //***********************

  isMaterialAllowed();
  setInitialCondition(_initial_cell_subdomain_id);
  updateSubdomainID(_allowed_parameter_values, _initial_cell_subdomain_id, _cell_subdomain_id);

  //*******************
  // Print the Results
  //*******************
  for (const auto & s : _cell_subdomain_id) // allowed starting from C++11
    std::cout << s << '\n';
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
  for (SubdomainName & element : _allowed_parameter_values)
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

// Set initial subdomain ids from the initial material used.
void
DiscreteOptimizationReporter::setInitialCondition(
    std::vector<SubdomainName> & initial_cell_subdomain_id)
{

  // Loop to add elements to _initial_cell_subdomain_id given number of elements
  for (dof_id_type i = 0; i < _total_cells; i++)
  {
    initial_cell_subdomain_id[i] = _initial_material_used;
  }
}

// Set cell subdomainIDs to a new one taking into account the previous subdomainIDs in
// the domain.
void
DiscreteOptimizationReporter::updateSubdomainID(
    const std::vector<SubdomainName> allowed_parameter_values,
    const std::vector<SubdomainName> previous_cell_subdomain_id,
    std::vector<SubdomainName> & cell_subdomain_id)
{
  if (cell_subdomain_id.size() != previous_cell_subdomain_id.size())
    mooseError("The sizes of the subdomainIDs do not match! Please make sure the previous "
               "subdomainIDs size is the same as the new one");

  // Setting the current Subdomain ID to the previous one.
  cell_subdomain_id = previous_cell_subdomain_id;

  // Seed the random number generator
  std::srand(std::time(0));

  //************************************************************************************************************************//

  // 1- Randomizing the current subdomain ID to a new one

  // Randomize the contents of cell_subdomain_id using "f" and "m" from _allowed_parameter_values
  // for (size_t i = 0; i < cell_subdomain_id.size(); i++)
  // {
  //   // Generate a random number between 0 and 1 only (hence "f" or "m").
  //   int random_index = std::rand() % 2;
  //   cell_subdomain_id[i] =
  //       _allowed_parameter_values[random_index]; // Assign either "f" or "m" randomly
  // }

  //************************************************************************************************************************//

  // 2- Randomizing the current subdomain ID based on some criterion

  // Define the threshold for the area of a circle
  // Real area_threshold = 7853.0;

  // // // // Randomize the contents of cell_subdomain_id based on the area of a circle
  // for (unsigned int i = 0; i < cell_subdomain_id.size(); i++)
  // {
  //   Real random_radius = std::rand() % 100; // Generate a random number between 0 and 99
  //   Real area = 3.141592 * random_radius * random_radius;

  //   if (area > area_threshold)
  //   {
  //     cell_subdomain_id[i] = "f";
  //   }
  //   else
  //   {
  //     cell_subdomain_id[i] = "m";
  //   }
  // }

  //************************************************************************************************************************//

  // 3- Using Simulated Annealing (Kinda?!)

  // Function to calculate the cost of a given Domain
  // Used to update the total_cost of the domain.

  // Simulated annealing parameters
  // There could be other parameters as well
  unsigned int iterations = 77;
  Real cooling_rate = 0.77;

  // initializing the temperature before the loop
  Real current_temperature = 77.0;

  for (unsigned int i = 0; i < iterations; i++)
  {
    // Generate a random neighbor by changing a random element in cell_subdomain_id
    std::vector<SubdomainName> neighbor = cell_subdomain_id;
    unsigned int random_index = std::rand() % neighbor.size();

    // "f" and "m" are at index 0 and 1 in _allowed_parameter_values, hence taking the
    // reminder division by 2, 0 or 1.
    unsigned int random_value_index = std::rand() % 2;

    neighbor[random_index] = allowed_parameter_values[random_value_index];

    // After assigning the neighbor element, we test this new domain (where just one element has
    // changed).

    Real current_cost = costFunction(cell_subdomain_id);
    Real neighbor_cost = costFunction(neighbor);

    // Calculate the cost difference
    Real cost_difference = neighbor_cost - current_cost;

    // If the neighbor has a lower cost, accept it
    if (cost_difference < 0)
    {
      cell_subdomain_id = neighbor;
    }
    else
    {
      // Otherwise, accept the neighbor with a probability depending on the temperature
      Real acceptance_probability = std::exp(-cost_difference / current_temperature);
      if ((std::rand() / static_cast<double>(RAND_MAX)) < acceptance_probability)
      {
        cell_subdomain_id = neighbor;
      }
    }

    // Cool down the temperature
    current_temperature *= cooling_rate;
  }
}

// Compute the cost function given some domain.
Real
DiscreteOptimizationReporter::costFunction(const std::vector<SubdomainName> & Domain)
{
  // Define our cost function here based on the Domain
  // For example, we can count the number of consecutive "f"s or "m"s!
  unsigned int total_cost = 0;

  // Starting from 1 since the first element in domain starts at 0 = i - 1.
  for (unsigned int i = 1; i < Domain.size(); i++)
  {
    if (Domain[i] == Domain[i - 1])
    {
      total_cost++;
    }
  }
  return total_cost;
}
