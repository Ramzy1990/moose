sink_temperature = 300
sink_htc = 1.2
thermal_conductivity_1 = 1
thermal_conductivity_2 = 10
source = 10

# [Mesh]
#   [cmg]
#     type = CartesianMeshGenerator
#     dim = 2
#     dx = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
#     dy = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
#     subdomain_id = '
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 2 1 1 1 1 1 1 2 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
#                     1 2 1 1 1 1 1 1 2 1 1 1 1 1 2
#                     2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
#                     2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
#                     2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
#                    '
#   []
# []

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1 1 1'
    subdomain_id = '
                    1 1 1 1
                    1 2 1 1
                    1 1 1 1
                    1 1 1 1
                   '
  []
[]

[Variables]
  [temperature]
    initial_condition = 10000
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = thermal_conductivity
  []

  [source]
    type = CoupledForce
    variable = temperature
    v = source_var
    block = '1'
  []

  [sink]
    type = CoupledForce
    variable = temperature
    v = sink_var
    block = '2'
  []
[]

[AuxVariables]
  [sink_var]
    family = MONOMIAL
    order = CONSTANT
    # block = 2
  []

  [source_var]
    family = MONOMIAL
    order = CONSTANT
    # block = 1
    initial_condition = ${source}
  []
[]

[AuxKernels]
  [sink_aux]
    type = ParsedAux
    variable = sink_var
    coupled_variables = 'temperature'
    expression = '-${sink_htc} * (temperature - ${sink_temperature})'
    block = 2
    # execute_on = FORWARD
  []
[]

[Materials]
  [block_1]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '${thermal_conductivity_1}'
    block = 1
  []

  [block_2]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '${thermal_conductivity_2}'
    block = 2
  []
[]

[Postprocessors]
  [total_source]
    type = ElementIntegralVariablePostprocessor
    variable = source_var
    block = 1
  []

  [max_temperature]
    type = ElementExtremeValue
    value_type = max
    variable = temperature
  []

  [cost_function]
    type = ParsedPostprocessor
    pp_names = 'total_source max_temperature'
    function = '-1*(total_source * if(max_temperature < 400, 1, 0.01))'
  []

[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-8
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
  csv = true
[]
