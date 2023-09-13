sink_temperature = 300
sink_htc = 1.2
thermal_conductivity_1 = 1
thermal_conductivity_2 = 10
source = 10

# [Mesh]
#   [cmg]
#     type = CartesianMeshGenerator
#     # elem_type = Tri3
#     dim = 2
#     dx = '2 2 2 2 2 2 2 2 2 2'
#     dy = '2 2 2 2 2 2 2 2 2 2'
#     ix = '2 2 2 2 2 2 2 2 2 2'
#     iy = '2 2 2 2 2 2 2 2 2 2'
#     subdomain_id = '
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 2 1 1 1 1 1
# 1 1 1 1 2 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1
#                   '
#   []
# []

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1 1'
    dy = '1 1 1 1 1'
    subdomain_id = '

2 2 2 2 2
2 2 2 2 2
2 2 1 2 2
2 2 2 2 2
2 2 2 2 2
                  '
  []
[]

[Variables]
  [temperature]
    initial_condition = 300
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
  # [total_source_1]
  #   type = Receiver
  # []
  [max_temperature]
    type = ElementExtremeValue
    value_type = max
    variable = temperature
  []

  # [cost_function]
  #   type = ParsedPostprocessor
  #   pp_names = 'total_source_1 max_temperature'
  #   function = '-1*(total_source_1 * if(max_temperature < 360, 1, 0.01))'
  #   # function = '-(total_source - (max_temperature / 2))'
  #   # function = 'max_temperature/total_source'
  #   #   # function = '1/(total_source * if(max_temperature > 405, 0.001, if(max_temperature < 395, 2, 1 - (max_temperature - 400) / 5 + 0.001)))'
  # []

  [cost_function]
    type = ParsedPostprocessor
    pp_names = 'total_source max_temperature'
    function = '-1*(total_source * if(max_temperature < 360, 1, 0.01))'
    # function = '-(total_source - (max_temperature / 2))'
    # function = 'max_temperature/total_source'
    #   # function = '1/(total_source * if(max_temperature > 405, 0.001, if(max_temperature < 395, 2, 1 - (max_temperature - 400) / 5 + 0.001)))'
  []

[]

# [MultiApps]
#   [forward1]
#     type = FullSolveMultiApp
#     input_files = sub_simple_heat_cool.i
#     clone_parent_mesh = true
#     # input_files = eigen_test.i
#     execute_on = 'TIMESTEP_BEGIN'
#     # execute_on = 'FORWARD'
#     #reset_apps = '0 0'
#     #reset_time = '1 2'
#     #execute_on = TIMESTEP_END
#     # execute_on = 'INITIAL TIMESTEP_BEGIN'
#   []
# []

# [Transfers]

#   # [mesh_transfer]
#   #   type = MultiAppMeshTransfer
#   #   to_multi_app = forward1
#   #   # from_multi_app = forward1
#   #   execute_on = 'TIMESTEP_BEGIN'
#   # []

#   [pp_transfer]
#     type = MultiAppPostprocessorTransfer
#     from_multi_app = forward1
#     reduction_type = maximum
#     from_postprocessor = total_source
#     to_postprocessor = total_source_1
#   []

# []

[Executioner]
  type = Steady
  # type = Transient
  nl_abs_tol = 1e-8
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
  csv = true
  # [console]
  # type = Console
  # print_mesh_changed_info = true
  # system_info = 'framework mesh aux nonlinear execution'
  # []
[]
