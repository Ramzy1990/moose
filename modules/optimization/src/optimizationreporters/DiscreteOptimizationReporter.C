//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteOptimizationReporter.h"

/// Has been used in the optimization app source files.
/// Seems to allow the "index_range" method found in the following file.
/// This seems to allow the range-based "for" loops used.
/// "index_range" returns the size of the passed vector.
/// Notice how we included this here instead of the declaration header file. We do not use
/// it there, but here.
#include "libmesh/int_range.h"

/// I guess here we are adding the Discrete optimization reporter to the optimization app.
/// This function is used in most of the C source files in MOOSE.
/// Adds a MooseObject to the registry with the given app name/label.  classname is the (unquoted)
/// c++ class.  Each object/class should only be registered once. Our Discrete optimization
/// class is under the Optimization App.
registerMooseObject("OptimizationApp", DiscreteOptimizationReporter);

/// Always start with the InputParameters class in your C source file in MOOSE. This
/// function is used in most of the C source files in MOOSE.
/// This are the validParams of the DiscreteOptimizationReporter class.
/// If you would like to add more classes parameters to your current class, then you use
/// something like this:
///"InputParameters params = StochasticReporter::validParams();
/// params += SurrogateModelInterface::validParams();
/// params += SamplerInterface::validParams();"
/// Notice how "validParams" function is used, the different classes names, the += operator, and the
/// params object of type(class) "InputParameters".

/// From where does the reporter class inherits its parameters? The optimize App inherits
/// from MOOSE App. An action inherits from Action. Optimization functions inherits from
/// Function. All of this behavior is seen in other modules where we inherit from MOOSE framework.

/// Our reporter should inherit the parameters from some class. The continous
/// OptimizationReporter class inherits from the "OptimizationReporterBase" class, which
/// in turns inherits from the "OptimizationData" class, which in turns inherits from the
/// "GeneralReporter" class. Hence, these last two classes could be a good candidate for
/// our new reporter class. Inspecting the OptimizationData class, we can find many
/// parameters that are associated with the inverse optimization. I guess this makes it
/// not a good candidate. It is then settled to use the GeneralReporter class! See? Very
/// simple! But a question comes: Q: What happens if we inherited validParams from some
/// place and end up not using all the parameters inherited? Is tha safe? (e.g., we
/// inherit the OptimizationData class and end up not using the parameters it provides).

/// We would like to make sure that the discrete optimization module is inlcuded in the
/// optimization app.

/// The optimization reporter is used inside of optimize solve. Hence this is how it is
/// included in the simulation of MOOSE! At long last this puzzle is solved!! This means
/// whatever variables and methods w einlcude here, it will be seen by the solver. The
/// solver also inherits from the MOOSE app and actions and whatever, so this part is
/// handeled by other source files and headers. We just then need to develop the reporter currently.
InputParameters
DiscreteOptimizationReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += ElementUserObject::validParams();
  params += OptimizationData::validParams();

  params.addClassDescription("Receives computed objective function, and contains reporters for "
                             "communicating between discreteOptimizeSolve and subapps.");

  /// The parameter names variable is just a holder to the names of the variables we would
  /// like to optimize. This is seen in the bimaterial test files (main.i). I guess we will
  /// need this one so I will keep it.
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");

  /// The number of variables is a holder to the number of variables adjusted under each
  /// parameter names we would like to optimize. This is seen in the bimaterial test files
  /// (main.i). I guess we will need this one so I will keep it.
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");

  /// E.g., we are adjusting the parameter_names=diffusivity_values, for two mateirals and
  /// hence num_values=2.
  /// Another example I think of would be parameter_names=k-eff, num_values=1 (since it is an
  /// integral variable).

  /// Initial condition for the parameter values. E.g.,  if we have 2 diffusivity_values
  /// (num_variables=2) then we would need initial_condition='3 4' (i.e., 2 values). This is
  /// seen in the bimaterial test files (main.i). I guess we will need this one so I will
  /// keep it. Furthermore, it seems to me the values here are actually real and not integer
  /// as we first postulated then (e.g., if we are optimizing the shape to get a good k-eff,
  /// we will need this to be real).
  params.addParam<std::vector<Real>>("initial_condition",
                                     "Initial condition for each parameter values, default is 0");

  /// lower bound for the parameter values. E.g.,  if we have 2 diffusivity_values
  /// (num_variables=2) then we would need lower_bounds='1 1' (i.e., 2 values). This is
  /// seen in the bimaterial test files (main.i). I guess we will need this one so I will
  /// keep it. Furthermore, it seems to me the values here are actually real and not integer
  /// as we first postulated then (e.g., if we are optimizing the shape to get a good k-eff,
  /// we would this to be real).
  params.addParam<std::vector<Real>>(
      "lower_bounds", std::vector<Real>(), "Lower bounds for each parameter value.");

  /// Upper bound for the parameter values. E.g.,  if we have 2 diffusivity_values
  /// (num_variables=2) then we would need upper_bounds='20 20' (i.e., 2 values). This is
  /// seen in the bimaterial test files (main.i). I guess we will need this one so I will
  /// keep it. Furthermore, it seems to me the values here are actually real and not integer
  /// as we first postulated then (e.g., if we are optimizing the shape to get a good k-eff,
  /// we would this to be real).
  params.addParam<std::vector<Real>>(
      "upper_bounds", std::vector<Real>(), "Upper bounds for each parameter value.");

  /// Now adding the variables I think we will need for the Discrete Optimization Reporter
  /// for shape optimization.

  return params;
}

// Here we have a list and this is why the separation is with ",". What we are doing here is
// that we are initializing the member variables of the "OptimizationReporterDiscrete"
// class. Those variables where declared in the header file.
// Subdomain IDs are the materials: Element 1 gets subdomain ID 1 or 2 or so on.
OptimizationReporterDiscrete::OptimizationReporterDiscrete(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparam(_parameter_names.size()),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0)),
    _lower_bounds(getParam<std::vector<int>>("lower_bounds")),
    _upper_bounds(getParam<std::vector<int>>("upper_bounds")),
// _adjoint_data(declareValueByName<std::vector<Real>>("adjoint", REPORTER_MODE_REPLICATED))

// Here we do some checks to make sure the values assigned or read make sense according to
// some logic.
{
  if (_parameter_names.size() != _nvalues.size())
    paramError("num_parameters",
               "There should be a number in 'num_parameters' for each name in 'parameter_names'.");

  std::vector<int> initial_condition = isParamValid("initial_condition")
                                           ? getParam<std::vector<int>>("initial_condition")
                                           : std::vector<int>(_ndof, 0);
  if (initial_condition.size() != _ndof)
    paramError("initial_condition",
               "Initial condition must be same length as the total number of parameter values.");

  if (_upper_bounds.size() > 0 && _upper_bounds.size() != _ndof)
    paramError("upper_bounds", "Upper bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() > 0 && _lower_bounds.size() != _ndof)
    paramError("lower_bounds", "Lower bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() != _upper_bounds.size())
    paramError((_lower_bounds.size() == 0 ? "upper_bounds" : "lower_bounds"),
               "Both upper and lower bounds must be specified if bounds are used");

  // Parameter values declared as reporter data. This is a variable declared in the header file.
  _parameters.reserve(_nparam);

  // Range-based for loop. Iterates over the indices of the "_parameter_names" vector using an
  // "index_range" object that returns the size of the passed vector.
  unsigned int v = 0;
  for (const auto i : index_range(_parameter_names))
  {
    _parameters.push_back(
        &declareValueByName<std::vector<int>>(_parameter_names[i], REPORTER_MODE_REPLICATED));
    _parameters[i]->assign(initial_condition.begin() + v,
                           initial_condition.begin() + v + _nvalues[i]);
    v += _nvalues[i];
  }
}

// The following are member functions of the discrete reporter class. They have their
// declarations in the header file.
void
OptimizationReporterDiscrete::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);

  // This nested loop is iterating over all of the values associated with each parameter in
  // the _parameters vector. param here is some variable just defined for the loop. It seems
  // to be a pointer?
  dof_id_type n = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
      x.set(n++, val);

  x.close();
}

void
OptimizationReporterDiscrete::updateParameters(const libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (auto & param : _parameters)
    for (auto & val : *param)
      val = x(n++);
}

// void
// OptimizationReporterDiscrete::computeGradient(libMesh::PetscVector<Number> & gradient) const
// {
//   if (_adjoint_data.size() != _ndof)
//     mooseError("Adjoint data is not equal to the total number of parameters.");

//   for (const auto i : make_range(_ndof))
//     gradient.set(i, _adjoint_data[i]);

//   gradient.close();
// }
