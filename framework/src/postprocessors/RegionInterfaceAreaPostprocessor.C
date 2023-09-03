//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RegionInterfaceAreaPostprocessor.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", RegionInterfaceAreaPostprocessor);

InputParameters
RegionInterfaceAreaPostprocessor::validParams()
{
  InputParameters params = InternalSidePostprocessor::validParams();
  params.addParam<std::vector<SubdomainName>>("primary_block_names",
                                              "The list of blocks on one side of the interface");
  params.addParam<std::vector<SubdomainName>>(
      "paired_block_names", "The list of blocks on the other side of the interface");
  params.addClassDescription("This postprocessor computes the surface area of the boundary between "
                             "two regions delineated by two sets of block ids");
  return params;
}

RegionInterfaceAreaPostprocessor::RegionInterfaceAreaPostprocessor(
    const InputParameters & parameters)
  : InternalSidePostprocessor(parameters), _area(0)
{
  if (isParamValid("primary_block_names"))
  {
    auto block_names = getParam<std::vector<SubdomainName>>("primary_block_names");
    auto vec_block_ids = _mesh.getSubdomainIDs(block_names);
    _primary_blk_ids.insert(vec_block_ids.begin(), vec_block_ids.end());
  }
  else
    _primary_blk_ids = _mesh.meshSubdomains();

  if (isParamValid("paired_block_names"))
  {
    auto block_names = getParam<std::vector<SubdomainName>>("paired_block_names");
    auto vec_block_ids = _mesh.getSubdomainIDs(block_names);
    _paired_blk_ids.insert(vec_block_ids.begin(), vec_block_ids.end());
  }
  else
    _paired_blk_ids = _mesh.meshSubdomains();
}

void
RegionInterfaceAreaPostprocessor::initialize()
{
  _area = 0;
}

void
RegionInterfaceAreaPostprocessor::execute()
{
  _area += computeIntegral();
}

Real
RegionInterfaceAreaPostprocessor::getValue()
{
  return _area;
}

void
RegionInterfaceAreaPostprocessor::threadJoin(const UserObject & y)
{
  const RegionInterfaceAreaPostprocessor & pps =
      static_cast<const RegionInterfaceAreaPostprocessor &>(y);
  _area += pps._area;
}

Real
RegionInterfaceAreaPostprocessor::computeIntegral()
{
  auto elem_subdomain_id = _current_elem->subdomain_id();
  auto neigh_subdomain_id = _neighbor_elem->subdomain_id();
  auto is_elem_id_in_blk = _primary_blk_ids.find(elem_subdomain_id) != _primary_blk_ids.end();
  auto is_elem_id_in_paired_blk = _paired_blk_ids.find(elem_subdomain_id) != _paired_blk_ids.end();
  auto is_neigh_id_in_blk = _primary_blk_ids.find(neigh_subdomain_id) != _primary_blk_ids.end();
  auto is_neigh_id_in_paired_blk =
      _paired_blk_ids.find(neigh_subdomain_id) != _paired_blk_ids.end();
  if ((is_elem_id_in_blk && is_neigh_id_in_paired_blk) ||
      (is_elem_id_in_paired_blk && is_neigh_id_in_blk))
    return _current_side_volume;
  return 0;
}

void
RegionInterfaceAreaPostprocessor::finalize()
{
  gatherSum(_area);
}
