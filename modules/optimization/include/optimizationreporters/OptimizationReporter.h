//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationReporterBase.h"

/**
 * Computes gradient and contains reporters for communicating between optimizeSolve and subapps
 */
class OptimizationReporter : public OptimizationReporterBase
{
public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

  virtual void setInitialCondition(libMesh::PetscVector<Number> & param) override;
  virtual void setInitialCondition(std::vector<int> & ix, std::vector<Real> & rx) override;
  virtual bool hasBounds() const override { return _upper_bounds.size() && _lower_bounds.size(); }
  virtual Real getUpperBound(dof_id_type i) const override { return _upper_bounds[i]; }
  virtual Real getLowerBound(dof_id_type i) const override { return _lower_bounds[i]; }
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const override;
  virtual dof_id_type getNumParams() const override { return _ndof; }
  virtual void updateParameters(const std::vector<int> & ix, const std::vector<Real> & rx) override;

protected:
  virtual void setICsandBounds() override;

private:
  virtual void setSimulationValuesForTesting(std::vector<Real> & data) override;
};
