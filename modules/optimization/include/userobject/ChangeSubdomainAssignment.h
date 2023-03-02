//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

class ChangeSubdomainAssignment : public ElementUserObject
{
public:
  static InputParameters validParams();

  ChangeSubdomainAssignment(const InputParameters & parameters);

  void initialize() override{};
  void execute() override;
  void threadJoin(const UserObject & /*uo*/) override{};
  void finalize() override;
  void initialSetup() override;

  ///@{ public interface to subdomain assignment
  std::map<dof_id_type, SubdomainID> subdomainAssignment() const { return _subdomain_assignment; }
  void setSubdomainAssignment(const std::map<dof_id_type, SubdomainID> & assignment) const;
  ///@}

protected:
  /// the current elem->subdomain assignment
  std::map<dof_id_type, SubdomainID> _subdomain_assignment;
};
