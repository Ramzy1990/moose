//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChangeSubdomainAssignment.h"

registerMooseObject("OptimizationApp", ChangeSubdomainAssignment);

InputParameters
ChangeSubdomainAssignment::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(".");

  return params;
}

ChangeSubdomainAssignment::ChangeSubdomainAssignment(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
  // this user object should not be block restricted because we'll
  // run into trouble when we change subdomain ids and then rely on
  // block restrictable info which cannot be updated.
  if (parameters.isParamSetByUser("block"))
    mooseError("This object cannot be block restricted.");
}

void
ChangeSubdomainAssignment::initialSetup()
{
  // get the subdomain ids and store them
  for (auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    _subdomain_assignment.insert(
        std::pair<dof_id_type, SubdomainID>(elem->id(), elem->subdomain_id()));

  // serialize this data for now, could be a memory issue by handling it in parallel
  // is painful
  // TODO
}

void
ChangeSubdomainAssignment::setSubdomainAssignment(
    const std::map<dof_id_type, SubdomainID> & assignment) const
{
  for (auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    auto p = assignment.find(elem->id());
    if (p != assignment.end())
      elem->subdomain_id() = p->second;
  }
  _mesh.update();
}

void
ChangeSubdomainAssignment::execute()
{
}

void
ChangeSubdomainAssignment::finalize()
{
}
