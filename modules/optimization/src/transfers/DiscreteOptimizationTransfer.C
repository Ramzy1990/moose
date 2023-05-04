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
// #include "ChangeSubdomainAssignment.h"

//*********************
// Regiester the MOOSE
//*********************
registerMooseObject("OptimizationApp", DiscreteOptimizationTransfer);

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

  // Set and suppress the 'execute_on' flag.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

//*******************
// Class Constructor
//*******************
DiscreteOptimizationTransfer::DiscreteOptimizationTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    // _user_object_name(getParam<UserObjectName>("user_object")),
    // _reporter(isParamValid("user_object")
    //               ? &getUserObject<DiscreteOptimizationReporter>("user_object")
    //               : nullptr)

    _reporter(_fe_problem.getUserObject<DiscreteOptimizationReporter>(
        getParam<UserObjectName>("user_object"))),

    // _reporter(getUserObject<DiscreteOptimizationReporter>("user_object")),

    // Getting the mesh from the "_to_problems" pointers.
    // .at() method throws an std::out_of_range exception if out of bounds.
    _to_mesh(_to_problems.at(0)->mesh()),

    // Getting the mesh from the "_from_problems" pointers.
    _from_mesh(_from_problems.at(0)->mesh())

// _reporter(getUserObject<DiscreteOptimizationReporter>("DiscreteOptimizationReporter"))

{

  // if (isParamValid("to_multi_app") && isParamValid("from_multi_app") &&
  //     getToMultiApp() != getFromMultiApp())
  //   paramError("to_multi_app",
  //              "Sibling multiapp transfer has not been implemented for this transfer.");

  // execute();
}

//***********************
// Functions Definitions
//***********************
void
DiscreteOptimizationTransfer::execute()
{
  TIME_SECTION(
      "DiscreteOptimizationTransfer::execute()", 5, "Performing transfer with a user object");

  getAppInfo();

  _skip = true;
  _it_transfer = 0;
  _it_transfer_to = 0;
  _it_transfer_from = 0;

  // Print the messages
  std::cout << "*** Welcome to the Discrete Shape Optimization Transfer! ***" << std::endl
            << std::endl;
  std::cout << "This marks the first invoking of this transfer class ..." << std::endl;
  std::cout << "Your user object name is: " << _reporter.name() << std::endl;
  std::cout << "Now we should log into the MultiApp system starting with the TO_MULTIAPP logical "
               "branch ..."
            << std::endl
            << std::endl;

  // _current_direction variable is found inside the MultiAppTransfer class.
  switch (_current_direction)
  {
    case TO_MULTIAPP:

      // iterators
      _it_transfer += 1;
      _it_transfer_to += 1;

      // Print the messages
      std::cout << "*** You are currently in the TO_MULTIAPP branch! ***" << std::endl << std::endl;
      std::cout << "The transfer iterations are as follow:-" << std::endl;
      std::cout << "Total Iterations:" << _it_transfer << std::endl;
      std::cout << "TO_MULTIAPP Iterations:" << _it_transfer_to << std::endl;

      // Return for initial invoking only
      if (_it_transfer_to == 1 || _skip == true)
      {
        std::cout << "This marks as the first invoking of the TO_MULTIAPP branch ..." << std::endl;
        return;
      }

      // Check if the _to_problem is a single problem
      if (_to_problems.size() > 1)
      {
        mooseError("The size of the _to_problem is more than one! "
                   "Please check the current discrete transfer capabilities.");
      }

      // for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
      // {
      // const auto * uo = &_to_problems[i_to]->getUserObjectBase(_user_object_name);
      //   const ChangeSubdomainAssignment * csa_uo =
      //       dynamic_cast<const ChangeSubdomainAssignment *>(uo);
      //   if (!csa_uo)
      //     paramError("user_object",
      //                "Must be present and also, most importantly a
      //                ChangeSubdomainAssignment");
      //   csa_uo->setSubdomainAssignment({{0, 10}, {2, 12}});
      // }
      // break;

      // add modification of the mesh.
      break;
    case FROM_MULTIAPP:

      // iterators
      _it_transfer += 1;
      _it_transfer_from += 1;

      // Print the messages
      std::cout << "*** You are currently in the TO_MULTIAPP branch! ***" << std::endl << std::endl;
      std::cout << "The transfer iterations are as follow:" << std::endl;
      std::cout << "Total Iterations:" << _it_transfer << std::endl;
      std::cout << "Total FROM_MULTIAPP Iterations:" << _it_transfer_from << std::endl;

      // Adjusting the _skip boolean for after initial calling of the TO_MULTIAPP branch.
      _skip = false;

      // Now we need to read the mesh from the MultiApp system. The reporter only sees the main app
      // mesh, but the mesh in the subapps are

      // -Initial assignment of the mesh and skip idea.- Compute the cost function.-

      // getReporter
      // get
      // _obj_function =
      // &_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");

      // here getSubdomainAssignment();
      break;
  }
}

// - Add here the cost function.
