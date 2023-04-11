//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChangeSubdomainTransfer.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "ChangeSubdomainAssignment.h"

registerMooseObject("OptimizationApp", ChangeSubdomainTransfer);

InputParameters
ChangeSubdomainTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject you want to transfer values from.  Note: This might be a "
      "UserObject from your MultiApp's input file!");

  params.addClassDescription("");
  return params;
}

ChangeSubdomainTransfer::ChangeSubdomainTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters), _user_object_name(getParam<UserObjectName>("user_object"))
{
  if (isParamValid("to_multi_app") && isParamValid("from_multi_app") &&
      getToMultiApp() != getFromMultiApp())
    paramError("to_multi_app",
               "Sibling multiapp transfer has not been implemented for this transfer.");
}

void
ChangeSubdomainTransfer::execute()
{
  TIME_SECTION("ChangeSubdomainTransfer::execute()", 5, "Performing transfer with a user object");

  getAppInfo();

  switch (_current_direction)
  {
    case TO_MULTIAPP:
      for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
      {
        const auto * uo = &_to_problems[i_to]->getUserObjectBase(_user_object_name);
        const ChangeSubdomainAssignment * csa_uo =
            dynamic_cast<const ChangeSubdomainAssignment *>(uo);
        if (!csa_uo)
          paramError("user_object",
                     "Must be present and also, most importantly a ChangeSubdomainAssignment");
        csa_uo->setSubdomainAssignment({{0, 10}, {2, 12}});
      }
      break;
    case FROM_MULTIAPP:
      // getReporter
      // get
      // _obj_function = &_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");

      // here getSubdomainAssignment();
      break;
  }
}
