//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVKernel.h"
#include "MooseVariableFV.h"
#include "MooseVariableInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"

/// FVElemental is used for calculating residual contributions from volume
/// integral terms of a PDE where the divergence theorem is not applied (e.g.
/// time derivative terms, source terms, etc.).  As with finite element
/// kernels, all solution values and material properties must be indexed using
/// the _qp member.  Note that all interfaces for finite volume kernels are
/// AD-based - be sure to use AD material properties and other AD values to
/// maintain good jacobian/derivative quality.
class FVElementalKernel : public FVKernel,
                          public MooseVariableInterface<Real>,
                          public CoupleableMooseVariableDependencyIntermediateInterface,
                          public MaterialPropertyInterface
{
public:
  static InputParameters validParams();
  FVElementalKernel(const InputParameters & parameters);

  /// Usually you should not override these functions - they have some
  /// tricky stuff in them that you don't want to mess up!
  ///@{
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian();
  ///@}

protected:
  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Note that solution gradients will be zero unless you are using a
  /// higher-order reconstruction.  Material properties and other values
  /// should be initialized just like they are for fintie element kernels here -
  /// since this is a FE-like volumetric integration term.
  virtual ADReal computeQpResidual() = 0;

  MooseVariableFV<Real> & _var;
  const ADVariableValue & _u;
  const unsigned int _qp = 0;
  const Elem * const & _current_elem;
};