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
#include "DiscreteOptimizationTransfer.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "DiscreteOptimizationReporter.h" // declared in the header file

//*********************
// Regiester the MOOSE
//*********************
// registerMooseObject("OptimizationApp", DiscreteOptimizationTransfer);
registerMooseObject("MooseApp", DiscreteOptimizationTransfer);

//*************************
// Helper Functions if Any
//*************************
// std::vector<VectorPostprocessorName>
// getVectorNamesHelper(const std::string & prefix, const std::vector<PostprocessorName> & pp_names)
// {
//   std::vector<VectorPostprocessorName> vec_names;
//   vec_names.reserve(pp_names.size());
//   for (const auto & pp_name : pp_names)
//   {
//     if (!prefix.empty())
//       vec_names.push_back(prefix + ":" + pp_name);
//     else
//       vec_names.push_back(pp_name);
//   }
//   return vec_names;
// }

//*************************
// Adding Input Parameters
//*************************
InputParameters
DiscreteOptimizationTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();

  // Class description
  params.addClassDescription(
      "This class transfer the mesh and related information from and to the multiapp system");

  // DiscreteOptimizationReporter user object
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject you want to transfer values from.  Note: This might be a UserObject from "
      "your MultiApp's input file!");

  // postprocessor cost function user object
  /// 📝 @TODO: I would like to restrict it to from multiapp only. Or we can suppress it and force its
  // name on the user.
  params.addRequiredParam<PostprocessorName>(
      "objective_name",
      "The objective function name as defined in the subapp postprocessors block. This "
      "function is the one you would like to transfer its value to the optimizer.  Note: This is a "
      "post processor name from your MultiApp's input file!");

  // Those are postprocessors that are related to the objective function components, so they will be
  // assigned from the postprocessors
  params.addParam<std::vector<PostprocessorName>>(
      "constraints_names",
      "These are the postprocessors names that you would like to set constraints upon. Note: These "
      "are postprocessors names from your MultiApps input file!. Another Note: The user should "
      "make sure the names of the postprocessors are those included in the objective "
      "functions!");

  params.addParam<std::vector<std::string>>(
      "inequality_operators",
      "These are the inequality operators associated with each postprocessor names that you would "
      "like to set constraints upon. Note: These "
      "are postprocessors names from your MultiApps input file!. Another Note: The user should "
      "make sure the names of the postprocessors are those included in the objective "
      "functions!");

  params.addParam<std::vector<PostprocessorValue>>(
      "constraints_values",
      "These are the constraints values you would like to set on your postprocessors names. Note: "
      "These are related to the postprocessors names and are in order of the"
      " 'constraints_names' parameter!");

  params.addParam<std::vector<std::string>>(
      "domain_constraints",
      "These are the constraints you can put on your doamin's mesh and they will be hardcoded in "
      "the optimizer.");

  // Set and suppress the 'execute_on' flag.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteOptimizationTransfer::DiscreteOptimizationTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters)

{
  // These variables are read only for the FROM_MULTIAPP
  if (getFromMultiApp())
  {

    // This variable in the from_multiapp will be used to get the value associated with it to the
    // objective function value by using the getPostprocessorValueByName
    _objective_name = isParamValid("objective_name") ? getParam<PostprocessorName>("objective_name")
                                                     : "objective";

    // Since those will not be used currently, as we need to know how they could be used, we wil
    // adjourn working on them currently:
    // These are the constraints which are the Postprocessors that are defined in the subapp and
    // should be constitutes of the objective function.
    if (isParamValid("constraints_names"))
      _constraints_names = getParam<std::vector<PostprocessorName>>("constraints_names");
    // else
    //   _transfer_constraints_names = {"c1"};

    if (isParamValid("inequality_operators"))
      _inequality_operators = getParam<std::vector<std::string>>("inequality_operators");

    // These are the constraints values and they will be used to limit the constituting components
    // of the objective function if needed.
    if (isParamValid("constraints_values"))
      _constraints_values = getParam<std::vector<PostprocessorValue>>("constraints_values");
    // else
    //   _transfer_constraints_values = {1.0};

    if (isParamValid("domain_constraints"))
      _domain_constraints = getParam<std::vector<std::string>>("domain_constraints");
  }
}

//***********************
// Functions Definitions
//***********************

void
DiscreteOptimizationTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  // Getting the user object
  auto & uo = _fe_problem.getUserObject<UserObject>(getParam<UserObjectName>("user_object"));
  _reporter = dynamic_cast<DiscreteOptimizationReporter *>(&uo);
  if (!_reporter)
    paramError("user_object", "This object must be a 'DiscreteOptimizationReporter' object.");

  // _skip = true;
  _it_transfer = 0;
  _it_transfer_to = 0;
  _it_transfer_from = 0;

  // getting the app information including the mesh and such
  // Not needed, included in the multiapptransfer initial setup
  // getAppInfo();

  // Now checking the if the constraints names provided have equivalent names in the subapps
  // Check that postprocessor on sub-application exists and create vectors on results VPP
  // const dof_id_type n = getFromMultiApp()->numGlobalApps();
  // for (MooseIndex(n) i = 0; i < n; i++)
  // {
  if (getFromMultiApp()->hasLocalApp(0))
  {

    FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(0);

    for (const auto & name : _constraints_names)
    {
      if (!app_problem.hasPostprocessorValueByName(name))
        mooseError("Unknown postprocesssor name '",
                   name,
                   "' on sub-application '",
                   getFromMultiApp()->name(),
                   "'");
    }

    if (!app_problem.hasPostprocessorValueByName(_objective_name))
      mooseError("Unknown objective function name '",
                 _objective_name,
                 "' on sub-application '",
                 getFromMultiApp()->name(),
                 "'");
  }
  // }
}

void
DiscreteOptimizationTransfer::execute()
{
  TIME_SECTION(
      "DiscreteOptimizationTransfer::execute()", 5, "Performing transfer with a user object");

  // getting the mesh
  // probably will need the _to_problems mesh only in release
  MooseMesh & to_mesh = _to_problems[0]->mesh();
  MooseMesh & from_mesh = _from_problems[0]->mesh();

  // general iteration
  _it_transfer += 1;

  // Important question, do we start executing the transfer from the start
  // everytime we solve the physics problem?? This is what happens currently

  // Print the messages once for the very first iteration:
  if (_it_transfer == 1)
  {
    std::cout << std::endl
              << "*** Welcome to the Discrete Shape Optimization Transfer! ***" << std::endl
              << std::endl;
    // std::cout << "This marks the first invoking of this transfer class ..." <<
    // std::endl;
    std::cout << "Your user object name is:  " << _reporter->name() << std::endl;
    std::cout << "Now we should log into the MultiApp system starting with the "
                 "TO_MULTIAPP logical branch to start the optimization process! Buckle up!..."
              << std::endl
              << std::endl;
  }

  // _current_direction variable is found inside the MultiAppTransfer class.
  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      _it_transfer_to += 1;

      std::cout << "\n*** You are currently in the TO_MULTIAPP branch! ***\n\n"
                << "The transfer iterations are as follow: \n"
                << "Total Iterations: " << _it_transfer << '\n'
                << "Total TO_MULTIAPP Iterations: " << _it_transfer_to << "\n\n";

      if (_it_transfer_to == 1) // First invocation
      {
        std::cout
            << "*** This marks as the first invoking of the TO_MULTIAPP branch ***\n\n"
            << "Nothing major will happen in the very first invoking of the TO_MULTIAPP branch...\n"
            << "Returning to the MULTIAPP system to continue the optimization process...\n\n";
        return;
      }
      else // Subsequent invocations
      {
        handleSubsequentToInvocations(to_mesh, _it_transfer_to);

        std::cout << "*** Finished the current handling of the TO_MULTIAPP branch! ***\n\n";
      }

      std::cout
          << "*** Great! See you on the other side then after the MultiApp solve! 👋 ***\n\n\n"
          << std::endl;

      break;
    }

    case FROM_MULTIAPP:
    {
      _it_transfer_from += 1;

      std::cout << "\n*** You are currently in the FROM_MULTIAPP branch! ***\n\n"
                << "The transfer iterations are as follow: \n"
                << "Total Iterations: " << _it_transfer << '\n'
                << "Total FROM_MULTIAPP Iterations: " << _it_transfer_from << "\n\n";

      if (_it_transfer_from == 1) // First invocation
      {
        handleFirstFromInvocation(from_mesh, _it_transfer_from);

        std::cout << "\n*** Finished the current handling of the FROM_MULTIAPP branch! ***\n\n";
      }
      else // Subsequent invocations
      {
        handleSubsequentFromInvocations(_it_transfer_from);

        std::cout << "\n*** Finished the current handling of the FROM_MULTIAPP branch! ***\n\n";
      }

      std::cout << "*** Great! See you on the other side then with a new global iteration!👋 "
                   "***\n\n\n\n\n\n\n"
                << std::endl;
      break;
    }
  }
}

void
DiscreteOptimizationTransfer::handleSubsequentToInvocations(MooseMesh & to_mesh,
                                                            dof_id_type & iteration)
{

  /// 📝 @TODO: Allow for multiphysics problems optimization! I think one can say
  // several to_problem meshes.

  if (_to_problems.size() > 1)
  {
    mooseError("The size of the _to_problem is more than one! "
               "Please check the current discrete transfer capabilities.");
  }

  std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n"
            << "*** Receiving the domain's mesh elements and subdomains ids ***\n"
            << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n\n";

  auto mesh_params = _reporter->getMeshParameters();
  _allowed_parameter_values = std::get<0>(mesh_params);
  _initial_pairs_to_optimize = std::get<1>(mesh_params);
  _pairs_to_optimize = std::get<2>(mesh_params);

  std::cout << "\nDomain's mesh content was received successfully for iteration " << iteration
            << "! ...\n\n";

  std::cout << "*** Updating the domain's mesh elements and subdomains ids and reassigning the "
            << "mesh ***\n\n";

  assignMesh(_pairs_to_optimize, to_mesh);

  std::cout << "*** New mesh to pass to the MultiApp: ***\n";

  // Print mesh elements to check
  printMeshElements(to_mesh);
}

void
DiscreteOptimizationTransfer::handleFirstFromInvocation(MooseMesh & from_mesh,
                                                        dof_id_type & iteration)
{
  std::cout << "*** This marks as the first invoking of the FROM_MULTIAPP branch ***\n";

  std::cout << "*** Current Mesh: ***\n";

  // Print mesh elements to check
  printMeshElements(from_mesh);

  std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***"
            << "\n*** Post processing of the mesh in the FROM_MULTIAPP branch ***"
            << "\n*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n\n";

  processAndVerifyMesh(from_mesh);

  _reporter->printCurrentDomain(iteration);

  std::cout << "*** Data is written and stored to a file successfully! ***\n\n";

  // Set the objective function value in the reporter
  objectiveFunctionPP(_objective_name, iteration);

  // Set the objective function components comparison results in the reporter
  buildAndComparePP(_constraints_names, _constraints_values, _inequality_operators, iteration);

  // Call the domainConstraints function to set the domain's mesh constraints in the reporter
  domainConstraints(_domain_constraints);
}

void
DiscreteOptimizationTransfer::handleSubsequentFromInvocations(dof_id_type & iteration)
{
  /// 📝 @TODO: Check if new materials are added.
  /// 📝 @TODO: Allow other materials to be present in the mesh even if not
  // optimized for from start, by adding them to the allowed material function.

  std::cout << "*** Obtaining the postprocessing parameters needed by the Discrete "
            << "Optimization Reporter for the objective function computation ***\n";

  _reporter->printCurrentDomain(iteration);

  std::cout << "*** Data is written and stored to a file successfully! ***\n\n";

  // Set the objective function value in the reporter
  objectiveFunctionPP(_objective_name, iteration);

  // Set the objective function components comparison results in the reporter
  buildAndComparePP(_constraints_names, _constraints_values, _inequality_operators, iteration);

  // No need to call the domainConstraints function
}

void
DiscreteOptimizationTransfer::assignMesh(
    const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize, MooseMesh & mesh)
{
  // elements owned by the processor (processor tag on it):
  // active_local_element_ptr_range()
  for (auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    auto p = pairs_to_optimize.find(elem->id());
    if (p != pairs_to_optimize.end())
      elem->subdomain_id() = p->second;
  }

  // Chaning the mesh and updating it
  mesh.meshChanged();

  std::cout << "*** Mesh Updated Successfully! ***\n\n";
}

void
DiscreteOptimizationTransfer::objectiveFunctionPP(const PostprocessorName & objective_name,
                                                  const dof_id_type & iteration)
{

  // Logging
  std::cout << "Starting objectiveFunctionPP with objective_name = " << objective_name
            << " and iteration = " << iteration << "\n";

  /// 📝 @TODO: Allow several apps as found in "SamplerPostprocessorTransfer::executeFromMultiapp()"
  // Retrieve the FEProblemBase instance
  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(0);

  // Get the value of the objective function 🎯
  PostprocessorValue objective_information;
  objective_information = app_problem.getPostprocessorValueByName(objective_name);

  // Logging
  std::cout << "Got objective result = " << objective_information << "\n";

  // Set the value in the reporter allowing for the optimizer to get it
  _reporter->setObjectiveInformation(objective_information, iteration);

  // Logging
  std::cout << "Finished setting objective information for iteration " << iteration << "\n\n";
}

void
DiscreteOptimizationTransfer::buildAndComparePP(
    const std::vector<PostprocessorName> & constraints_names,
    const std::vector<PostprocessorValue> & constraints_values,
    const std::vector<std::string> & inequality_operators,
    const dof_id_type & iteration)
{

  // Check that the constraints names and values vectors match in size
  if (constraints_names.size() != constraints_values.size() ||
      constraints_names.size() != inequality_operators.size())
  {
    throw std::runtime_error(
        "Mismatched vector sizes! Please revise the sizes of your input constraints vectors.");
  }

  /// 📝 @TODO: Allow several apps as found in "SamplerPostprocessorTransfer::executeFromMultiapp()"

  // Retrieve the FEProblemBase instance
  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(0);

  // Build the pp_map using getPostprocessorValueByName method from the app_problem
  std::map<PostprocessorName, PostprocessorValue> pp_map;
  for (const auto & name : constraints_names)
  {
    pp_map[name] = app_problem.getPostprocessorValueByName(name);
  }

  // Build the constraints_map and inequality_operators_map
  std::map<PostprocessorName, PostprocessorValue> constraints_map;
  std::map<PostprocessorName, std::string> inequality_operators_map;
  for (std::size_t i = 0; i < constraints_names.size(); i++)
  {
    constraints_map[constraints_names[i]] = constraints_values[i];
    inequality_operators_map[constraints_names[i]] = inequality_operators[i];
  }

  // Call the comparePP function and store its results as boolean
  std::vector<bool> comparison_results =
      comparePP(pp_map, constraints_map, inequality_operators_map);

  // Now we can use comparison_results internally in the buildAndComparePP function
  printComparisonTable(comparison_results, pp_map, constraints_map, constraints_names, iteration);

  // // For example, let us print the results:
  // for (std::size_t i = 0; i < comparison_results.size(); i++)
  // {
  //   PostprocessorName constraint_name = constraints_names[i];
  //   std::cout << "Constraint " << constraint_name
  //             << " passed: " << std::boolalpha // to print true or false and not 1 or 0.
  //             << comparison_results[i] << std::endl;
  // }

  // // Print the maps for debugging, some good visuals, and to follow the process
  // std::cout << "Postprocessor values:" << std::endl;
  // printMap(pp_map);
  // std::cout << "Constraint values:" << std::endl;
  // printMap(constraints_map);

  // Now we pass the comparison results to the reporter for further actions
  _reporter->setConstraintsComparisonInformation(comparison_results);
}

std::vector<bool>
DiscreteOptimizationTransfer::comparePP(
    const std::map<PostprocessorName, PostprocessorValue> & postprocessors_values,
    const std::map<PostprocessorName, PostprocessorValue> & constraints_values,
    const std::map<PostprocessorName, std::string> & inequality_operators)
{

  std::vector<bool> results;

  // Loop over the constraint values
  for (const auto & constraint_entry : constraints_values)
  {
    const auto & constraint_name = constraint_entry.first;
    auto constraint_value = constraint_entry.second;

    // Check if there's a corresponding postprocessor value
    auto pp_iter = postprocessors_values.find(constraint_name);
    if (pp_iter == postprocessors_values.end())
    {
      throw std::runtime_error("Missing postprocessor value for " + constraint_name +
                               "! Please check your input and try again.");
    }
    auto pp_value = pp_iter->second;

    // Check if there's a corresponding inequality operator
    auto inequality_iter = inequality_operators.find(constraint_name);
    if (inequality_iter == inequality_operators.end())
    {
      throw std::runtime_error("Missing inequality operator for " + constraint_name +
                               "! Please check your input and try again.");
    }
    auto inequality = inequality_iter->second;

    // Perform the comparison based on the inequality operator
    if (inequality == "<")
    {
      results.push_back(pp_value < constraint_value);
    }
    else if (inequality == ">")
    {
      results.push_back(pp_value > constraint_value);
    }
    else if (inequality == "<=")
    {
      results.push_back(pp_value <= constraint_value);
    }
    else if (inequality == ">=")
    {
      results.push_back(pp_value >= constraint_value);
    }
    else
    {
      throw std::runtime_error("Unknown inequality operator for " + constraint_name + ": " +
                               inequality +
                               "! Please use the following inequalities only: >, <, >=, <= ");
    }
  }

  // The results vector will be cleared everytime we log into this function, no need for clear() if
  // I correctly understand.
  return results;
}

void
DiscreteOptimizationTransfer::printComparisonTable(
    const std::vector<bool> & comparison_results,
    const std::map<PostprocessorName, PostprocessorValue> & postprocessors_values,
    const std::map<PostprocessorName, PostprocessorValue> & constraints_values,
    const std::vector<PostprocessorName> & constraints_names,
    const dof_id_type & iteration)
{
  std::cout << "+----------------------------+------------------+------------------+---------+"
            << std::endl;
  std::cout << "| " << std::setw(77) << std::left << "iteration " << iteration << " |" << std::endl;
  std::cout << "+----------------------------+------------------+------------------+---------+"
            << std::endl;
  std::cout << "| " << std::setw(26) << std::left << "Constraint Name"
            << " | " << std::setw(16) << "PP Value"
            << " | " << std::setw(14) << "Constraint Value"
            << " | " << std::setw(7) << "Result"
            << " |" << std::endl;
  std::cout << "+----------------------------+------------------+------------------+---------+"
            << std::endl;
  for (std::size_t i = 0; i < comparison_results.size(); i++)
  {
    PostprocessorName constraint_name = constraints_names[i];
    PostprocessorValue pp_value = postprocessors_values.at(constraint_name);
    PostprocessorValue constraint_value = constraints_values.at(constraint_name);
    std::cout << "| " << std::setw(26) << std::left << constraint_name << " | " << std::setw(16)
              << pp_value << " | " << std::setw(14) << constraint_value << " | " << std::setw(7)
              << std::boolalpha << comparison_results[i] << " |" << std::endl;
  }
  std::cout << "+----------------------------+------------------+------------------+---------+"
            << std::endl;
}

void
DiscreteOptimizationTransfer::domainConstraints(const std::vector<std::string> & domain_constraints)
{
  // Create a set of allowed constraints
  std::set<std::string> allowed_constraints = {
      "moderator_at_boundaries", "volume_preserved", "neighbour_same_type"};

  // Check if the vector is empty
  if (domain_constraints.empty())
  {
    std::cout << "No domain constraints provided.\n";
  }
  else
  {
    // Print the domain constraints
    std::cout << "Domain Constraints: \n";
    for (const auto & constraint : domain_constraints)
    {
      // Check if the constraint is allowed
      if (allowed_constraints.find(constraint) == allowed_constraints.end())
      {
        // If the constraint is not allowed, print an error message
        mooseError(
            "Error: Invalid domain constraint '",
            constraint,
            "'. Current allowed constraints are 'moderator_at_boundaries', 'volume_preserved', "
            "and 'neighbour_same_type'.");
      }

      // If the constraint is allowed, print it
      std::cout << constraint << std::endl;
    }
  }

  // Set the domain constraints in the reporter object
  _reporter->setDomainConstraints(domain_constraints);
}

void
DiscreteOptimizationTransfer::printMap(const std::map<PostprocessorName, PostprocessorValue> & map)
{
  if (map.empty())
  {
    std::cout << "The map you would like to print is empty! " << std::endl;
    return;
  }

  // Find the longest key to calculate the width of the output
  std::size_t max_key_length = 0;
  for (const auto & pair : map)
  {
    if (pair.first.size() > max_key_length)
    {
      max_key_length = pair.first.size();
    }
  }

  // Create a format string for the output
  // The width of the first column is max_key_length + 4, to add some padding
  std::string format = "%-" + std::to_string(max_key_length + 4) + "s %f\n";

  // Print the contents of the map
  for (const auto & pair : map)
  {
    printf(format.c_str(), pair.first.c_str(), pair.second);
  }
}

void
DiscreteOptimizationTransfer::printMeshElements(MooseMesh & mesh)
{
  // Loop over elements
  for (auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    std::cout << "Element ID: " << std::setw(7) << std::left << elem->id()
              << " Material ID: " << std::setw(7) << std::left << elem->subdomain_id() << "\n";
  }
  std::cout << std::endl;
}

void
DiscreteOptimizationTransfer::processAndVerifyMesh(MooseMesh & mesh)
{
  std::cout << "\nObtaining and verifying the mesh before post-processing...\n\n";

  std::cout << "Verifying if the materials utilized in the problem match those allowed for "
               "optimization\n";
  _reporter->isMaterialAllowed(mesh);

  std::cout << "\nAcquiring the first-mesh domain elements and subdomain IDs\n";
  _reporter->setInitialCondition(mesh);

  std::cout << "\nSuccessful acquisition of the domain! The Discrete Optimization reporter now has "
               "the initial mesh information!\n\n";
}
