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
#include "DiscreteOptimizationTransferTest.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "DiscreteOptimizationReporter.h" // declared in the header file
// #include "ChangeSubdomainAssignment.h"

//*********************
// Regiester the MOOSE
//*********************
registerMooseObject("OptimizationApp", DiscreteOptimizationTransferTest);

//*************************
// Adding Input Parameters
//*************************
InputParameters
DiscreteOptimizationTransferTest::validParams()
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
DiscreteOptimizationTransferTest::DiscreteOptimizationTransferTest(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters)

{
}

//***********************
// Functions Definitions
//***********************

void
DiscreteOptimizationTransferTest::initialSetup()
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
  // getAppInfo();
}

void
DiscreteOptimizationTransferTest::execute()
{
  TIME_SECTION(
      "DiscreteOptimizationTransferTest::execute()", 5, "Performing transfer with a user object");

  // getting the mesh
  // probably will need the _to_problems mesh only in release
  MooseMesh & to_mesh = _to_problems[0]->mesh();
  MooseMesh & from_mesh = _from_problems[0]->mesh();

  // general iteration
  _it_transfer += 1;

  // Important question, do we start executing the transfer from the start everytime we
  // solve the physics problem??
  // This is what happens currently

  // Print the messages
  std::cout << std::endl
            << "*** Welcome to the Discrete Shape Optimization Transfer! ***" << std::endl
            << std::endl;
  // std::cout << "This marks the first invoking of this transfer class ..." << std::endl;
  std::cout << "Your user object name is:  " << _reporter->name() << std::endl;
  std::cout << "Now we should log into the MultiApp system starting with the TO_MULTIAPP logical "
               "branch..."
            << std::endl
            << std::endl;

  // _current_direction variable is found inside the MultiAppTransfer class.
  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      // iterators
      _it_transfer_to += 1;

      // Print the messages
      std::cout << "*** You are currently in the TO_MULTIAPP branch! ***" << std::endl << std::endl;
      std::cout << "The transfer iterations are as follows:" << std::endl;
      std::cout << "TOTAL Iterations:" << _it_transfer << std::endl;
      std::cout << "TO_MULTIAPP Iterations:" << _it_transfer_to << std::endl;

      // Return to the multiapp system for initial invoking only
      if (_it_transfer_to == 1)
      {

        // Steps 1, 2, and 3!
        // Step 1: is going to the TO multiapp system.
        // Step 2: is skipping doing anything here.
        // Step 3: is solving the multipapp system physics problem.
        // Go to the FROM branch for Step 4.

        std::cout << std::endl
                  << "*** This marks as the first invoking of the TO_MULTIAPP branch ***"
                  << std::endl
                  << std::endl;
        std::cout
            << "Nothing major will happen in the very first invoking of the TO_MULTIAPP branch..."
            << std::endl;
        std::cout << "Returning to the MULTIAPP system to continue the optimization process..."
                  << std::endl;
        return;
      }

      // Check if the _to_problem is a single problem. Print error if not, for now.
      // TODO: Allow for multiphysics problems optimization! I think one can say several to_problem
      // meshes.

      if (_to_problems.size() > 1)
      {
        mooseError("The size of the _to_problem is more than one! "
                   "Please check the current discrete transfer capabilities.");
      }

      // Step 10: In the TO_problem, we get the updated mesh domain, assign the mesh,
      // making it ready for the solve step.

      std::cout << std::endl
                << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***" << std::endl
                << "*** Receiving the domain's mesh elements and subdomains ids ***" << std::endl
                << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***" << std::endl
                << std::endl;

      // For now, we just change the mesh domain randomly before passing it to the MULTIAPP system.
      // FIXME: This is for testing only. The updateSubdomainID updates the mesh vectors and maps
      // based on some random work.

      // First we access the passed values through the _reporter object using a TUBLE getter
      // function.
      // This is not needed for the testing phase!
      // auto & [_transfer_allowed_parameter_values,
      //         _transfer_initial_pairs_to_optimize,
      //         _transfer_pairs_to_optimize] = _reporter->getMeshParameters();

      auto mesh_params = _reporter->getMeshParameters();
      _transfer_allowed_parameter_values = std::get<0>(mesh_params);
      _transfer_excluded_parameter_values = std::get<1>(mesh_params);
      _transfer_initial_pairs_to_optimize = std::get<2>(mesh_params);
      _transfer_pairs_to_optimize = std::get<3>(mesh_params);

      std::cout << std::endl
                << "Domain's mesh content was received successfully! ... " << std::endl
                << std::endl;

      std::cout << "*** Updating the domain's mesh elements and subdomains ids ***" << std::endl
                << "CAUTION! This is for testing purposes only! Otherwise, we assign the mesh "
                   "directly ..."
                << std::endl;

      // Next we update the subdomain ID in the reporter based on the supplied mesh (FOR Testing
      // only!):
      _reporter->updateSubdomainID(_transfer_allowed_parameter_values,
                                   _transfer_initial_pairs_to_optimize,
                                   _transfer_pairs_to_optimize,
                                   to_mesh);

      // Q: Confirm if the variables will be udpated accordingly or not based on reference the way
      // they are currently.

      // Next we assign the mesh:

      std::cout << "*** Great! Now reassigning the Mesh ***" << std::endl << std::endl;

      assignMesh(_transfer_pairs_to_optimize, to_mesh);

      for (auto & elem : to_mesh.getMesh().active_local_element_ptr_range())
      {
        std::cout << "Element ID: " << elem->id() << " Material ID: " << elem->subdomain_id()
                  << std::endl;
      }

      // The reporter should have the optimizer's domain constraints (e.g., moderator specific
      // locations) set by a user and pass it to the optimizer (an executioner) (TODO). This is
      // different from the cost function constraints (TODO)!
      // TODO: Add specifics for the elements on the boundary if needed.

      // if (_it_transfer == 10)
      //   exit(EXIT_SUCCESS);

      break;
    }
    case FROM_MULTIAPP:
    {

      // iterators
      _it_transfer_from += 1;

      // Print the messages
      std::cout << "*** You are currently in the FROM_MULTIAPP branch! ***" << std::endl
                << std::endl;
      std::cout << "The transfer iterations are as follow:" << std::endl;
      std::cout << "Total Iterations:" << _it_transfer << std::endl;
      std::cout << "Total FROM_MULTIAPP Iterations:" << _it_transfer_from << std::endl;

      std::cout << " " << std::endl;
      std::cout << "*** Current FROM Mesh: ***" << std::endl;
      for (auto & elem : from_mesh.getMesh().active_local_element_ptr_range())
      {
        std::cout << "Element ID: " << elem->id() << " Material ID: " << elem->subdomain_id()
                  << std::endl;
      }

      // We need to read the mesh from the MultiApp system. The reporter only sees the main app
      // mesh, but the mesh in the subapps are the one actually used by the problem solver.

      // TODO: Check if new materials are added.
      // TODO: Allow other materials to be present in the mesh even if not optimized for from start,
      // by adding them to the allowed material function.

      // TODO: Add a function to read any constraint added by the user imposed on the domain
      // reconstruction (but domain construction does not happen in the "from" branch). Could
      // be a set of options that the user can choose from. This should be in the reporter and
      // passed over to the optimizer.

      // TODO: Save unperturbed domain results to a file for comparisons.
      // TODO: Add a parameter to enable testing the multiapp system optimization outside of the
      // release version.

      std::cout << std::endl;
      std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***" << std::endl;
      std::cout << "*** Post processing of the mesh in the FROM_MULTIAPP branch ***" << std::endl;
      std::cout << "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***" << std::endl;
      std::cout << std::endl;

      if (_it_transfer_from == 1)
      {

        // Step 4: We get the mesh for the first time, check it, and get the domain map.

        std::cout << "*** Initiating the first invocation of the FROM_MULTIAPP branch ***"
                  << std::endl;
        std::cout << std::endl;

        // Obtain the mesh and verify its integrity before performing post-processing
        std::cout << "Obtaining and verifying the mesh before post-processing..." << std::endl;
        std::cout << std::endl;

        std::cout
            << "*** Verifying if the materials utilized in the problem match those allowed for "
               "optimization ***"
            << std::endl;

        _reporter->isMaterialAllowed(from_mesh);

        std::cout << std::endl
                  << "*** Acquiring the first-mesh domain elements and subdomain IDs ***"
                  << std::endl;

        _reporter->setInitialCondition(from_mesh);

        std::cout << "Successful acquirment of the domain! The Discrete Optimization reporter now "
                     "has the initial mesh information!..."
                  << std::endl
                  << std::endl;

        // Step 5: We post process the initial mesh and write out the results to
        // a file for comparison down the road!

        std::cout << "*** Now postporcessing and storing the mesh data and the SubApp results in a "
                     "file ***"
                  << std::endl
                  << std::endl;

        // TODO: Add here the printing out of the mesh information and the results while
        // postprocessing the data through postprocessors, such that the reporter will have all the
        // information needed to compute the cost function, whenever needed!

        //  --->        // Q: How to post_process?

        // Maybe we can printout the results follwoing the post process step such that we print
        // them out in one step

        _reporter->printCurrentDomain(_it_transfer_from);

        // std::cout << "*** Data is written and stored to a file successfully! ***" << std::endl;
        // std::cout << std::endl;

        // Step 6: Pass on this initial mesh postprocessing results to the reporter.
        // The reporter then will have these reuslts ready to compute the initial cost function.

        // Add here sending the post processed information to the reporter.
        // Currently, nothing happens here in the testing phase!
        //  --->        _reporter.setDomainPostProcessInformation(_it_transfer_from);

        // Step 7: The reporter should pass the mesh data to the optimizer. The reporter should be
        // called inside the optimizer (I guess through an object) to get its data.

        // Step 8: In the optimizer, we solve and optimize the passed mesh domain. This happens
        // inside the wrapper part of the optimizer.

        // We do NOT compute the cost function just yet like they did with the old optimization
        // domain!!

        // Step 9: Instead, we then update the mesh parameters, i.e., map, in the reporter.

        // Step 10: We then go to the TO_problem, to get the updated mesh domain (map), assign the
        // mesh, making it ready for the solve step.

        // This passing is done through the reporter, the only thing that connects the transfer and
        // the executioner classes.

        std::cout << "*** Now returning to the MULTIAPP System (and the next global iteration) ***"
                  << std::endl
                  << std::endl
                  << std::endl
                  << std::endl;

        return;

      } // if (_it_transfer_from == 1)

      // Obtain the elements and subdomains of the current mesh targeted for optimization.
      // This step establishes the domain for the subsequent solution stages.

      std::cout << "*** Obtaining the domain mesh elements and subdomains ids ***" << std::endl;
      std::cout << "CAUTION! For testing purposes only! Otherwise, we should proceed to "
                   "postprocess the results for cost function computation ... "
                << std::endl;

      // sending the mesh and everything is being taken care of on the reporter side during the
      // test phase only!
      _reporter->setInitialCondition(from_mesh);

      std::cout << "*** Obtaining the postprocessing parameters needed by the Discrete "
                   "Optimization Reporter for the cost function computation ***"
                << std::endl;

      // Step 11: After the physics problem is solved, we post process the results, pass it to the
      // reporter to compute the new cost function.

      // Note that we do not need to pass the mesh as it is the same one that the reporter already
      // have and have been acquired by the TO multiapp system! UNLESS the mesh has changed in the
      // physics solving section due to some adaptive mesh refinement or whatever.

      // Lather, Rinse, Repeat until some constraint is achieved  as provided by the user for the
      // cost function!

      // TODO: Postprocessing the data through postprocessors.
      // Postprocessors should call setters to set the data in the reporter class, I think.
      // Or we can just pass the referenced variables through one function to the reporter to
      // assign its variables.

      // Get information from the physics problem and to use for our future objective function.
      // Post processing on the mesh.
      // User defined names and post processors.

      // cost function is computed in the reporter side.

      std::cout << "*** Now returning to the MULTIAPP System (and the next global iteration) ***"
                << std::endl
                << std::endl
                << std::endl
                << std::endl;

      break;
    }
  }
}

void
DiscreteOptimizationTransferTest::assignMesh(
    const std::map<dof_id_type, subdomain_id_type> & pairs_to_optimize, MooseMesh & mesh)
{
  // elements owned by the processor (processor tag on it): active_local_element_ptr_range()
  for (auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    auto p = pairs_to_optimize.find(elem->id());
    if (p != pairs_to_optimize.end())
      elem->subdomain_id() = p->second;
  }
  // _to_problems[0]->mesh().reinit();
  // _to_problems[0]->es().reinit();
  // _to_problems[0]->mesh().prepare();
  // mesh.prepare();

  // We've performed some mesh adaptivity. We need to
  // clear any quadrature nodes such that when we build the boundary node lists in
  // MooseMesh::meshChanged we don't have any extraneous extra boundary nodes lying around
  // mesh.clearQuadratureNodes();
  // mesh.meshChanged();
  // _to_problems[0]->mesh().clearQuadratureNodes();

  // _to_problems[0]->mesh().meshChanged();
  mesh.meshChanged();

  // _to_problems[0]->mesh().update();
  // _to_problems[0]->mesh().meshChanged();

  std::cout << "*** Mesh Updated Successfully! ***" << std::endl << std::endl;
}
