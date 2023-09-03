//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSidePostprocessor.h"

/**
 * This postprocessor computes the surface area of the boundary between two regions delineated by
 * two sets of block ids
 */
class RegionInterfaceAreaPostprocessor : public InternalSidePostprocessor
{
public:
  static InputParameters validParams();

  RegionInterfaceAreaPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeFaceInfoIntegral(const FaceInfo * /* fi */)
  {
    mooseError("Integral over faces have not been implemented for this postprocessor");
  };
  virtual Real computeIntegral();

  Real _area;
  std::set<SubdomainID> _primary_blk_ids;
  std::set<SubdomainID> _paired_blk_ids;
};
