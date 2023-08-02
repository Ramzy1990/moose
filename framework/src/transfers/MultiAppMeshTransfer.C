//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Associated h-file include
#include "MultiAppMeshTransfer.h"

// MOOSE includes
#include "MultiApp.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", MultiAppMeshTransfer);

InputParameters
MultiAppMeshTransfer::validParams()
{

  InputParameters params = MultiAppTransfer::validParams();

  params.addClassDescription("Transfer meshes between multiapps.");

  return params;
}

MultiAppMeshTransfer::MultiAppMeshTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters)
// : MultiAppTransfer(parameters),
// _subapp_index(getParam<unsigned int>("subapp_index"))
{
}

void
MultiAppMeshTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  // _skip = true;
  _it_transfer = 0;
  _it_transfer_to = 0;
  _it_transfer_from = 0;
}

void
MultiAppMeshTransfer::execute()
{
  // TIME_SECTION("MultiAppMeshTransfer::execute()", 1, "Copies mesh!");
  // getAppInfo();

  // Perform action based on the transfer direction
  switch (_current_direction)
  {
    // main app to multi_app
    case TO_MULTIAPP:
    {

      // general iteration
      _it_transfer += 1;

      // Important question, do we start executing the transfer from the start
      // everytime we solve the physics problem?? This is what happens currently

      // Print the messages once for the very first iteration:
      if (_it_transfer == 1 && hasToMultiApp())
      {

        // if (_debug_on)
        // {

        std::cout << std::endl
                  << "*** Welcome to the MultiApp Mesh Transfer! ***" << std::endl
                  << std::endl;
        // std::cout << "This marks the first invoking of this transfer class ..." <<
        // std::endl;
        std::cout << "Now we should log into the MultiApp system starting with the "
                     "TO_MULTIAPP logical branch to start the transfer process! Buckle up!..."
                  << std::endl
                  << std::endl;
        // }
      }

      _it_transfer_to += 1;

      // if (_debug_on)
      // {
      std::cout << "\n*** You are currently in the TO_MULTIAPP branch! ***\n\n"
                << "The transfer iterations are as follow: \n"
                << "Total Iterations: " << _it_transfer << '\n'
                << "Total TO_MULTIAPP Iterations: " << _it_transfer_to << "\n\n";
      // }
      if (_it_transfer_to == 1) // First invocation
      {
        std::cout
            << "*** This marks as the first invoking of the TO_MULTIAPP branch ***\n\n"
            << "Nothing major will happen in the very first invoking of the TO_MULTIAPP branch...\n"
            << "Returning to the MULTIAPP system to continue the optimization process...\n\n\n\n";
        return;
      }
      // _fe_problem.mesh().getMesh().write("mesh_before_transfer.e");
      // Extract the mesh that is being transferred
      // FEProblemBase & from_problem = getFromMultiApp()->problemBase();
      // _fe_problem is the current app mesh.

      MooseMesh & from_mesh = _fe_problem.mesh();
      // MeshBase & from_libmesh = from_problem.mesh().getMesh();

      /// @brief a variable to store mesh map of the main app.
      std::map<dof_id_type, subdomain_id_type> mesh_map;

      for (auto & elem : from_mesh.getMesh().active_local_element_ptr_range())
      {
        // Populate the mesh map
        mesh_map.insert(
            std::pair<dof_id_type, subdomain_id_type>(elem->id(), elem->subdomain_id()));
      }

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        if (getToMultiApp()->hasLocalApp(i))
        {

          // Get reference to the mesh that will be written
          // appProblem is used similar to problemBase, but with the subapp index.
          FEProblemBase & to_problem = getToMultiApp()->appProblemBase(i);
          MooseMesh & to_mesh = to_problem.mesh();
          // MeshBase & to_libmesh = to_problem.mesh().getMesh();

          // Check if the meshes are identical. This is required for the transfer to work in
          // discrete mesh transfer, where we do not change the nodes or elements of the mesh.
          // You have to use a libmesh to access the nodes and elements, using the getMesh method.
          if ((to_mesh.getMesh().n_nodes() != to_mesh.getMesh().n_nodes()) ||
              (to_mesh.getMesh().n_elem() != to_mesh.getMesh().n_elem()))
            mooseError(
                "The meshes must be identical, in the number of elements and number of nodes, to "
                "utilize MultiAppMeshTransfer!");

          // Trying to replicate the clone_parent_mesh
          // Not needed.
          // InputParameters app_params = AppFactory::instance().getValidParams(_app_type);
          // app_params.set<const MooseMesh *>("_master_mesh") = &_fe_problem.mesh();

          // Now changing the subdomain_IDs of the subApp mesh from the main app mesh map
          for (auto & elem : to_mesh.getMesh().active_local_element_ptr_range())
          {
            auto p = mesh_map.find(elem->id());
            if (p != mesh_map.end())
              elem->subdomain_id() = p->second;
          }

          // Changing the mesh and updating it
          // to_mesh.meshChanged();
        }
      }

      break;
    }

    // multi_app to main app
    case FROM_MULTIAPP:
    {
      break;

      // if (!transfers_done)
      //   mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp
      //   "
      //              "per multiapp involved on each rank");
    }

      // multi_app to multi_app
      // case BETWEEN_MULTIAPP:
      // {
      //   break;
      //   // if (!transfers_done)
      //   //   mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
      //   //              "per multiapp involved on each rank");
      // }
  }
}
