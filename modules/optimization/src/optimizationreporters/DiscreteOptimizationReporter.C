//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/// 📝 @TODO: Add a value for the cost function after which the solution that satisfies it will stop. If left blank, then do not stop.

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
  InputParameters params = GeneralReporter::validParams();

  params.addClassDescription("Receives computed objective function, and contains reporters for "
                             "communicating between discreteOptimizeSolve and subapps.");

  /// Allowed values for my cell_subdomain_ID (e.g., materials possible to use {f,m,v}).
  params.addParam<std::vector<subdomain_id_type>>(
      "allowed_mateirals",
      "Allowed values for the subdomain_ID (e.g., materials possible to use {0,1,2}).");

  /// Allowed values for my cell_subdomain_ID (e.g., materials possible to use {f,m,v}).
  params.addParam<std::vector<subdomain_id_type>>(
      "excluded_materials",
      "excluded values for the subdomain_ID (e.g., materials not possible to use {0,1,2}).");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteOptimizationReporter::DiscreteOptimizationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{

  if (isParamValid("allowed_mateirals"))
    _allowed_parameter_values = getParam<std::vector<subdomain_id_type>>("allowed_mateirals");
  else
    _allowed_parameter_values = {0, 1};

  if (isParamValid("excluded_materials"))
    _excluded_parameter_values = getParam<std::vector<subdomain_id_type>>("excluded_materials");
  else
    _excluded_parameter_values = {10000, 10001};

  //***********************
  // Calling The Functions
  //***********************
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

  if (_fe_problem.hasMultiApps())
  {
    std::cout << "***Discrete Optimization Reporter: SubApp Mode***" << std::endl;

    /// 📝 @TODO: make allowed materials stand for the excluded materials.
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
    std::cout << "Verification done successfully!...\n\n";
  }
}

// Function to get the optimization domain of the problem using the elements of the mesh
void
DiscreteOptimizationReporter::getOptimizationDomain(const MooseMesh & domain_mesh)
{
  std::cout << "Assigning the elements and subdomains ids to their respective map from the mesh..."
            << std::endl
            << "***Elements-IDs are the keys, while subdomain-IDs are the corresponding values***"
            << std::endl;

  // Check the multi-app condition before the loop to avoid checking it for each element
  // bool is_multi_apps = _fe_problem.hasMultiApps();

  // Clear previous assignments for pairs_to_optimize
  _pairs_to_optimize.clear();

  for (auto & elem : domain_mesh.getMesh().active_local_element_ptr_range())
  {
    // Always add the pairs to optimize
    _pairs_to_optimize.insert(
        std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
  }

  std::cout << "Assignment done successfully for MultiApp case!...\n\n";
}

// Set initial subdomain ids from the initial material used, or from the domain's mesh
void
DiscreteOptimizationReporter::setInitialCondition(const MooseMesh & domain_mesh)
{
  if (_fe_problem.hasMultiApps())
  {
    std::cout << "*** Discrete Optimization Reporter: SubApp Mode ***" << std::endl;

    getOptimizationDomain(domain_mesh);

    _initial_pairs_to_optimize = _pairs_to_optimize;

    std::cout << "After initialization, the domain pairs to optimize are:\n";
    printMap(_pairs_to_optimize);
  }
}

void
DiscreteOptimizationReporter::setObjectiveInformation(const PostprocessorName & objective_name,
                                                      const Real & objective_information,
                                                      const dof_id_type & iteration)
{
  // Logging
  if (iteration == 1)
  {
    std::cout << "Setting objective information with objective_information = "
              << objective_information << " and iteration = " << iteration << std::endl;
  }

  // Set the objective information for this iteration
  _objective_result = objective_information;
  _objective_name = objective_name;
}

Real
DiscreteOptimizationReporter::getObjectiveResult() const
{
  return _objective_result;
}

PostprocessorName
DiscreteOptimizationReporter::getObjectiveName() const
{
  return _objective_name;
}

void
DiscreteOptimizationReporter::printCurrentDomain(const dof_id_type & iteration)
{
  std::ofstream file;
  std::string fileName;
  std::string directoryPath =
      "/home/ramzy/projects/moose/modules/combined/test/tests/DiscreteOptimization/";

  fileName = "Current_Domain.txt";

  // Open the file. Use std::ios::app to append to existing file
  file.open(directoryPath + fileName, std::ios::app);

  // If the file can not be opened, log an error message and return
  if (!file.is_open())
  {
    std::cout << "Failed to open file at path: " << directoryPath + fileName << std::endl;
    return;
  }

  // Print a separator with iteration number
  file << "Iteration " << iteration << ":" << std::endl;

  // Print the map
  for (const auto & pair : _pairs_to_optimize)
  {
    file << "Key: " << std::setw(6) << std::left << pair.first << ", Value: " << pair.second
         << std::endl;
  }

  file << "-------------------------------------------------------" << std::endl;

  // Close the file
  file.close();
}

void
DiscreteOptimizationReporter::setMeshDomain(const std::vector<int> & optimized_vector)
{
  // Initialize the converted vector
  std::vector<subdomain_id_type> converted_vector;

  // Check that the size of the input vector matches the size of the keys in the map
  if (optimized_vector.size() != _pairs_to_optimize.size())
  {
    mooseError("Size of the input vector does not match the size of the map keys!");
  }

  // Iterate over the input vector
  for (int val : optimized_vector)
  {
    // Check if conversion would overflow
    if (val > std::numeric_limits<subdomain_id_type>::max())
    {
      // If so, throw an error
      mooseError("Int value cannot be converted to subdomain_id_type without overflow!");
    }

    // Safely convert int to subdomain_id_type and add to the new vector
    converted_vector.push_back(static_cast<subdomain_id_type>(val));
  }
  // Assuming _pairs_to_optimize is the map with correct keys
  _pairs_to_optimize = valuesVectorToMap(_pairs_to_optimize, converted_vector);
}

void
DiscreteOptimizationReporter::getMeshDomain(std::vector<int> & iparams,
                                            std::vector<Real> & rparams,
                                            std::vector<int> & exec_params)
{
  std::vector<subdomain_id_type> originalVector = mapToValuesVector(_pairs_to_optimize);
  iparams.clear(); // Clear the original vector
  iparams.reserve(originalVector.size());

  exec_params.clear(); // Clear the original vector
  exec_params.reserve(_excluded_parameter_values.size());

  for (subdomain_id_type val : originalVector)
  {

    // This may potentially lose information if the subdomain_id_type is larger than what an int can
    // hold!! Not to mention it is unsigned while int is signed!
    /// 📝 @TODO: make sure the type is set to subdomain_ID in the optimizer or use integer here instead of subdomain_id
    iparams.push_back(static_cast<int>(val));
  }

  for (subdomain_id_type val : _excluded_parameter_values)
  {
    // This may potentially lose information if the subdomain_id_type is larger than what an int can
    // hold!! Not to mention it is unsigned while int is signed!
    /// 📝 @TODO: make sure the type is set to subdomain_ID in the optimizer or use integer here instead of subdomain_id
    exec_params.push_back(static_cast<int>(val));
  }

  rparams = {};
}

std::tuple<std::vector<subdomain_id_type> &,
           std::vector<subdomain_id_type> &,
           std::map<dof_id_type, subdomain_id_type> &,
           std::map<dof_id_type, subdomain_id_type> &>
DiscreteOptimizationReporter::getMeshParameters()
{
  return std::tie(_allowed_parameter_values,
                  _excluded_parameter_values,
                  _initial_pairs_to_optimize,
                  _pairs_to_optimize);
}

void
DiscreteOptimizationReporter::printMap(
    const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize)
{
  std::ostringstream keysStream;
  std::ostringstream valuesStream;

  // Limit on number of elements to print.
  const std::size_t printLimit = 10;

  // Find the length of the longest element ID
  int max_length = 0;
  for (const auto & pair : pairs_to_optimize)
  {
    int length = std::to_string(pair.first).length();
    if (length > max_length)
    {
      max_length = length;
    }
  }

  // Now print the element IDs and subdomain IDs with appropriate spacing
  std::size_t count = 0;
  for (const auto & pair : pairs_to_optimize)
  {
    // Only print up to the limit at the start and end of the map.
    if (count < printLimit || count >= pairs_to_optimize.size() - printLimit)
    {
      keysStream << std::setw(max_length) << pair.first << " ";
      valuesStream << std::setw(max_length) << pair.second << " ";
    }
    else if (count == printLimit)
    {
      // Indicate that some elements were omitted.
      keysStream << "... ";
      valuesStream << "... ";
    }
    ++count;
  }

  std::cout << "Element IDs:  " << keysStream.str() << std::endl;
  std::cout << "Subdomain IDs:" << valuesStream.str() << std::endl << std::endl;
}
