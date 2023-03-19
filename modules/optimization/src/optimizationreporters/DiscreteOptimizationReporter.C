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
#include "DiscreteOptimizationReporter.h"

/// Has been used in the optimization app source files.
/// Seems to allow the "index_range" method found in the following file.
/// This seems to allow the range-based "for" loops used.
/// "index_range" returns the size of the passed vector.
/// Notice how we included this here instead of the declaration header file. We do not use
/// it there, but here.
#include "libmesh/int_range.h"

//*********************
// Regiester the MOOSE
//*********************
/// I guess here we are adding the Discrete optimization reporter to the optimization app.
/// This function is used in most of the C source files in MOOSE.
/// Adds a MooseObject to the registry with the given app name/label.  classname is the (unquoted)
/// c++ class.  Each object/class should only be registered once. Our Discrete optimization
/// class is under the Optimization App.
registerMooseObject("OptimizationApp", DiscreteOptimizationReporter);

//*************************
// Adding Input Parameters
//*************************
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
  // params += OptimizationData::validParams(); // I do not think we need this? It has parameters
  // for measurment values and such.

  params.addClassDescription("Receives computed objective function, and contains reporters for "
                             "communicating between discreteOptimizeSolve and subapps.");

  /// Those will be used using the "getParam" object.

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");

  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");

  params.addParam<std::string>("assign_type",
                               "String to decide the assign type for the cells pattern. Manual "
                               "where we assign manually the cells pattern. Automatic where "
                               "we assign using the mesh.");

  /// Allowed values for my cell_subdomain_ID (e.g., materials possible to use {f,m,v}).
  params.addParam<std::set<std::string>>(
      "allowed_values",
      "Allowed values for my region_ID (e.g., materials possible to use {f,m,v}).");

  /// Initial condition for the parameter values. E.g., the subdomain id used for all the
  /// cells of the domain.
  params.addParam<std::string>(
      "initial_material",
      "The initial material we are going to assign to all the cells in the domain.");

  /// Assign the subdomain ID type that is going to be optimized.
  // Let us assign it from the initial cell_subdomain_id.

  /// Assign the subdomain ID type that we would like to reach (for test purposes and learning).
  params.addParam<std::vector<std::vector<std::string>>>("target_cell_subdomain_id",
                                                         "The subdomain ID type that is required.");

  /// Assign the cell pattern id that is going to be optimized.
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "cell_pattern", "The cell pattern (cell ids) that is going to be optimized.");

  return params;
}

//**************************
// Getting Input Parameters
//**************************
// Here we have a list and this is why the separation is with ",". What we are doing here is
// we are initializing the member variables of the "DiscreteOptimizationReporter"
// class. Those variables where declared in the header file. Some of them are read from
// the ".i" file. Subdomain IDs are the materials: Element 1 gets subdomain ID "f" or "m" or so on.
DiscreteOptimizationReporter::DiscreteOptimizationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // Getting our data from the "".i" file
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparam(_parameter_names.size()),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0)),

    _assign_type(getParam<std::string>("assign_type")),
    _allowed_parameter_values(getParam<std::string>("allowed_values")),
    _initial_material_used(getParam<std::string>("initial_material")),
    _cell_subdomain_id(getParam<std::vector<std::vector<std::string>>>("target_cell_subdomain_id")),
    _cell_pattern(getParam<std::vector<std::vector<dof_id_type>>>("cell_pattern"))

//**********************
// Checking Values Read
//**********************
// Here we do some checks to make sure the values assigned or read make sense according to
// some logic.

// add one for the allowed parameter values checking that the assigned subdoamin id is
// included in it.
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

  // Will be assigned from the "_initial_material_used" variable.
  // _initial_cell_subdomain_id = _initial_cell_subdomain_id
}

//********************************
// Working on the Parameters Read
//********************************
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
