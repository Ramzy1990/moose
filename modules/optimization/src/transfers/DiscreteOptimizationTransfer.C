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
// No helper methods to use

//*******************************
// Adding validParameters method
//*******************************
InputParameters
DiscreteOptimizationTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();

  // Class description
  params.addClassDescription("This class transfers the configuration from the optimizer to the "
                             "multiapp system and updates the mesh.");

  // DiscreteOptimizationReporter user object
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject, the reporter, you want to transfer values from and to. Note: This might be "
      "a UserObject from "
      "your MultiApp's input file!");

  // postprocessor cost function user object
  /// 📝 @TODO: I would like to restrict it to from multiapp only. Or we can suppress it and force its
  // name on the user.
  params.addRequiredParam<PostprocessorName>(
      "objective_name",
      "The objective function name as defined in the subapp postprocessors block. This "
      "function is the one you would like to transfer its value to the optimizer.  Note: This is a "
      "post processor name from your MultiApp's input file!");

  // Turning debugging on or off
  params.addParam<bool>("debug",
                        "The debug variable to print more information during the transfer. If on, "
                        "then we print several "
                        "messages during the transfer.");

  // Set and suppress the 'execute_on' flag.
  params.set<ExecFlagEnum>("execute_on") = {/*EXEC_INITIAL,*/ EXEC_TIMESTEP_BEGIN};
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
  // if (getFromMultiApp())
  // {

  // Debug variable, defaults to off
  _debug_on = isParamValid("debug") ? getParam<bool>("debug") : 0;

  // This variable in the from_multiapp will be used to get the value associated with it to the
  // objective function value by using the getPostprocessorValueByName
  _objective_name = isParamValid("objective_name") ? getParam<PostprocessorName>("objective_name")
                                                   : "cost_function";
  // }
}

//***********************
// Functions Definitions
//***********************

void
DiscreteOptimizationTransfer::initialSetup()
{
  // TIME_SECTION("DiscreteOptimizationTransfer::initialSetup()",
  //  2,
  //  "Initial setup for performing transfer with a user object");

  MultiAppTransfer::initialSetup(); // Using the initial setup of the MultiAppTransfer class

  // Getting the user object
  auto & uo = _fe_problem.getUserObject<UserObject>(getParam<UserObjectName>("user_object"));
  _reporter = dynamic_cast<DiscreteOptimizationReporter *>(&uo);
  if (!_reporter)
    paramError("user_object", "This object must be a 'DiscreteOptimizationReporter' object.");

  _it_transfer = 0;
  _it_transfer_to = 0;
  _it_transfer_from = 0;

  // getting the app information including the mesh and such:
  // Not needed, included in the multiapptransfer initial setup!
  // getAppInfo();

  // Now checking the if the constraints names provided have equivalent names in the subapps
  // Check that postprocessor on sub-application exists and create vectors on results VPP
  // const dof_id_type n = getFromMultiApp()->numGlobalApps();
  // for (MooseIndex(n) i = 0; i < n; i++)
  // {

  // No need to check the other sub-apps. We will only check the first one for the objective
  // function name. Checking other subapps do not make sense and will only cause the user to
  // write the objective function name in all subapps!
  if (getMultiApp()->hasLocalApp(0))
  {

    FEProblemBase & app_problem = getMultiApp()->appProblemBase(0);

    if (!app_problem.hasPostprocessorValueByName(_objective_name))
      mooseError("Unknown objective function name '",
                 _objective_name,
                 "' on sub-application '",
                 getMultiApp()->name(),
                 "'");
  }
  // }
}

void
DiscreteOptimizationTransfer::execute()
{
  TIME_SECTION(
      "DiscreteOptimizationTransfer::execute()", 3, "Performing transfer with a user object");

  // getting the mesh
  // probably will need the _to_problems mesh only in release
  // const dof_id_type n = getFromMultiApp()->numGlobalApps(); // Maybe the number of subapps do
  // not coincide with the number of _to_problems

  // Single sub-app!
  // MooseMesh & to_mesh = _to_problems[0]->mesh();
  // MooseMesh & from_mesh = _from_problems[0]->mesh();

  // Multi sub-apps!
  std::vector<MooseMesh *> to_meshes(_to_problems.size());
  std::vector<MooseMesh *> from_meshes(_from_problems.size());

  // std::cout << "*** Number of subApps ***" << std::endl << _to_problems.size() << std::endl;

  for (unsigned int i = 0; i < _to_problems.size(); i++)
  {
    to_meshes[i] = &_to_problems[i]->mesh();
    from_meshes[i] = &_from_problems[i]->mesh();
  }

  MooseMesh & to_mesh = *to_meshes[0];
  MooseMesh & from_mesh = *from_meshes[0];

  // general iteration
  _it_transfer += 1;

  // Important question, do we start executing the transfer from the start
  // everytime we solve the physics problem?? This is what happens currently

  // Print the messages once for the very first iteration:
  if (_it_transfer == 1 && hasToMultiApp())
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

      if (_debug_on)
      {
        std::cout << "\n*** You are currently in the TO_MULTIAPP branch! ***\n\n"
                  << "The transfer iterations are as follow: \n"
                  << "Total Iterations: " << _it_transfer << '\n'
                  << "Total TO_MULTIAPP Iterations: " << _it_transfer_to << "\n\n";
      }
      if (_it_transfer_to == 1) // First invocation
      {
        std::cout << "*** This marks as the first invokation of the TO_MULTIAPP branch ***\n\n"
                  << "Nothing major will happen in the very first invoking of the TO_MULTIAPP "
                     "branch...\n"
                  << "Returning to the MULTIAPP system to continue the optimization "
                     "process...\n\n\n\n";
        return;
      }
      else // Subsequent invocations
      {
        handleSubsequentToInvocations(to_mesh, to_meshes, _it_transfer_to);

        if (_debug_on)
        {

          std::cout << "*** Finished the current handling of the TO_MULTIAPP branch! ***\n\n";
        }
      }
      if (_debug_on)
      {

        std::cout << "*** Great! See you on the other side after the MultiApp solve! 👋 ***\n\n\n"
                  << std::endl;
      }
      break;
    }

    case FROM_MULTIAPP:
    {
      _it_transfer_from += 1;

      if (_debug_on)
      {

        std::cout << "\n\n\n*** You are currently in the FROM_MULTIAPP branch! ***\n\n"
                  << "The transfer iterations are as follow: \n"
                  << "Total Iterations: " << _it_transfer << '\n'
                  << "Total FROM_MULTIAPP Iterations: " << _it_transfer_from << "\n\n";
      }
      if (_it_transfer_from == 1) // First invocation
      {
        handleFirstFromInvocation(from_mesh, _it_transfer_from);
        if (_debug_on)
        {

          std::cout << "\n*** Finished the current handling of the FROM_MULTIAPP branch! ***\n\n";
        }
      }
      else // Subsequent invocations
      {
        handleSubsequentFromInvocations(_it_transfer_from);
        if (_debug_on)
        {

          std::cout << "\n*** Finished the current handling of the FROM_MULTIAPP branch! ***\n\n";
        }
      }
      if (_debug_on)
      {

        std::cout << "*** Great! See you on the other side with a new global iteration! 👋 "
                     "***\n\n\n\n\n\n\n\n\n\n"
                  << std::endl;
      }
      break;
    }
  }
}

void
DiscreteOptimizationTransfer::handleSubsequentToInvocations(MooseMesh & to_mesh,
                                                            std::vector<MooseMesh *> & to_meshes,
                                                            dof_id_type & iteration)
{

  if (_debug_on)
  {

    std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n"
              << "*** Receiving the domain's mesh elements and subdomains ids ***\n"
              << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n\n";

    std::cout << "Starting to receive the domain's previous mesh content ...\n";
  }
  auto mesh_params = _reporter->getMeshParameters();
  // _allowed_parameter_values = std::get<0>(mesh_params); // Not needed
  // _excluded_parameter_values = std::get<1>(mesh_params); // Not needed
  // _initial_pairs_to_optimize = std::get<2>(mesh_params); // Not needed
  _pairs_to_optimize = std::get<2>(mesh_params);

  if (_debug_on)
  {

    std::cout << "Domain's mesh content was received successfully for iteration: " << iteration
              << "! ...\n";

    std::cout << "Updating the domain's mesh elements-IDs and subdomains-IDs and reassigning the "
              << "mesh ...\n\n";
  }

  assignMesh(_pairs_to_optimize, to_meshes);

  if (_debug_on)
  {

    std::cout << "*** New mesh to pass to the MultiApp ***\n";

    // Print mesh elements to check
    printMeshElements(to_mesh);
  }
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

  std::cout << "Storing Data to a file ... \n";
  _reporter->printCurrentDomain(iteration);
  std::cout << "Data is written and stored to a file successfully! ...\n\n";

  // Set the objective function value in the reporter
  std::cout << "Now setting the optimization function value in the reporter ... \n";
  objectiveFunctionPP(_objective_name, iteration);
  std::cout << "Objective function acquired successfully! ...\n\n";
  // // Set the objective function components comparison results in the reporter
  // buildAndComparePP(_constraints_names, _constraints_values, _inequality_operators,
  // iteration);

  // // Call the domainConstraints function to set the domain's mesh constraints in the reporter
  // domainConstraints(_domain_constraints);
}

void
DiscreteOptimizationTransfer::handleSubsequentFromInvocations(dof_id_type & iteration)
{
  /// 📝 @TODO: Check if new materials are added.
  /// 📝 @TODO: Allow other materials to be present in the mesh even if not
  // optimized for from start, by adding them to the allowed material function.
  if (_debug_on)
  {
    std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***"
              << "\n*** Post processing of the mesh in the FROM_MULTIAPP branch ***"
              << "\n*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n\n";

    std::cout << "Storing Data to a file ... \n";
    _reporter->printCurrentDomain(iteration);
    std::cout << "Data is written and stored to a file successfully! ...\n\n";
  }

  // Set the objective function value in the reporter
  objectiveFunctionPP(_objective_name, iteration);

  // Set the objective function components comparison results in the reporter
  // buildAndComparePP(_constraints_names, _constraints_values, _inequality_operators,
  // iteration);

  // No need to call the domainConstraints function
}

void
DiscreteOptimizationTransfer::assignMesh(
    const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize,
    std::vector<MooseMesh *> & meshes)
{
  for (MooseMesh * mesh : meshes)
  {
    // elements owned by the processor (processor tag on it):
    // active_local_element_ptr_range()
    for (auto & elem : mesh->getMesh().active_local_element_ptr_range())
    {
      auto p = pairs_to_optimize.find(elem->id());
      if (p != pairs_to_optimize.end())
        elem->subdomain_id() = p->second;
    }

    // Changing the mesh and updating it
    mesh->meshChanged();
  }

  if (_debug_on)
  {
    std::cout << "*** Mesh Updated Successfully! ***\n\n";
  }
}

void
DiscreteOptimizationTransfer::objectiveFunctionPP(const PostprocessorName & objective_name,
                                                  const dof_id_type & iteration)
{

  // Logging
  if (_debug_on == 1 || iteration == 1)
  {
    std::cout << "Starting objectiveFunctionPP with objective_name = " << objective_name
              << " and iteration = " << iteration << "\n";
  }

  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(0);

  // Get the value of the objective function 🎯
  PostprocessorValue objective_information;
  objective_information = app_problem.getPostprocessorValueByName(objective_name);

  // Logging
  if (_debug_on == 1 || iteration == 1)
  {
    std::cout << "Got objective result 🎯 = " << objective_information << "\n";
  }
  // Set the value in the reporter allowing for the optimizer to get it
  _reporter->setObjectiveInformation(objective_name, objective_information, iteration);

  // Logging
  if (_debug_on == 1 || iteration == 1)
  {
    std::cout << "Finished setting objective information for iteration 🧮: " << iteration << "\n\n";
  }
}

// void
// DiscreteOptimizationTransfer::printComparisonTable(
//     const std::vector<bool> & comparison_results,
//     const std::map<PostprocessorName, PostprocessorValue> & postprocessors_values,
//     const std::map<PostprocessorName, PostprocessorValue> & constraints_values,
//     const std::vector<PostprocessorName> & constraints_names,
//     const dof_id_type & iteration)
// {
//   std::cout << "+----------------------------+------------------+------------------+---------+"
//             << std::endl;
//   std::cout << "| " << std::setw(77) << std::left << "iteration " << iteration << " |" <<
//   std::endl; std::cout <<
//   "+----------------------------+------------------+------------------+---------+"
//             << std::endl;
//   std::cout << "| " << std::setw(26) << std::left << "Constraint Name"
//             << " | " << std::setw(16) << "PP Value"
//             << " | " << std::setw(14) << "Constraint Value"
//             << " | " << std::setw(7) << "Result"
//             << " |" << std::endl;
//   std::cout << "+----------------------------+------------------+------------------+---------+"
//             << std::endl;
//   for (std::size_t i = 0; i < comparison_results.size(); i++)
//   {
//     PostprocessorName constraint_name = constraints_names[i];
//     PostprocessorValue pp_value = postprocessors_values.at(constraint_name);
//     PostprocessorValue constraint_value = constraints_values.at(constraint_name);
//     std::cout << "| " << std::setw(26) << std::left << constraint_name << " | " << std::setw(16)
//               << pp_value << " | " << std::setw(14) << constraint_value << " | " << std::setw(7)
//               << std::boolalpha << comparison_results[i] << " |" << std::endl;
//   }
//   std::cout << "+----------------------------+------------------+------------------+---------+"
//             << std::endl;
// }

// void
// DiscreteOptimizationTransfer::printMap(const std::map<PostprocessorName, PostprocessorValue> &
// map)
// {
//   if (map.empty())
//   {
//     std::cout << "The map you would like to print is empty! " << std::endl;
//     return;
//   }

//   // Find the longest key to calculate the width of the output
//   std::size_t max_key_length = 0;
//   for (const auto & pair : map)
//   {
//     if (pair.first.size() > max_key_length)
//     {
//       max_key_length = pair.first.size();
//     }
//   }

//   // Create a format string for the output
//   // The width of the first column is max_key_length + 4, to add some padding
//   std::string format = "%-" + std::to_string(max_key_length + 4) + "s %f\n";

//   // Print the contents of the map
//   for (const auto & pair : map)
//   {
//     printf(format.c_str(), pair.first.c_str(), pair.second);
//   }
// }

void
DiscreteOptimizationTransfer::printMeshElements(MooseMesh & mesh)
{

  // mesh map for printing
  std::map<dof_id_type, subdomain_id_type> meshmap;
  meshmap.clear();

  // Loop over elements
  for (auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    std::cout << "Element ID: " << std::setw(7) << std::left << elem->id()
              << " Material ID: " << std::setw(7) << std::left << elem->subdomain_id() << "\n";

    meshmap[elem->id()] = elem->subdomain_id();

    // Always add the pairs to optimize
    // _pairs_to_optimize.insert(
    // std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
  }
  std::cout << std::endl;
  _reporter->printMap(meshmap);
}

void
DiscreteOptimizationTransfer::processAndVerifyMesh(MooseMesh & mesh)
{
  std::cout << "Obtaining and verifying the mesh before post-processing...\n";

  std::cout << "Verifying if the materials utilized in the problem match those allowed for "
               "optimization\n";
  _reporter->isMaterialAllowed(mesh);

  std::cout << "Acquiring the original configuration's domain elements and subdomain IDs\n";
  _reporter->setInitialCondition(mesh);

  std::cout << "Successful acquisition of the domain! The Discrete Optimization reporter now has "
               "the initial configuration information!\n\n";
}
