# density_f = 2.146e-3 # kg cm-3
k_f = 0.205 # W cm-1 K-1
k_m = 0.100 # W cm-1 K-1
sink_htc = 100
sink_temperature = 600

[GlobalParams]
  #   use_exp_form = false
  # temperature = temp
  # integrate_p_by_parts = true
[]

[Mesh]
  #   file = pin_cell_mesh.e
  [cmg]
    type = CartesianMeshGenerator
    # elem_type = Tri3
    dim = 2
    dx = '0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063'
    dy = '0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063 0.063'
    # dz = '0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126'
    # dz = '0.126 0.126 0.126 0.126 0.126'
    # ix = '2 2 2 2 2 2 2 2 2 2'
    # iy = '2 2 2 2 2 2 2 2 2 2'
    ix = '4 4 4 4 4 4 4 4 4 4'
    iy = '4 4 4 4 4 4 4 4 4 4'
    subdomain_id = '

 0 0 0 0 0 0 0 0 0 0
 0 0 0 0 0 0 0 0 0 0
 1 1 1 1 1 1 1 1 0 0
 1 1 1 1 1 1 1 1 0 0
 1 1 1 1 1 1 1 1 0 0
 1 1 1 1 1 1 1 1 0 0
 1 1 1 1 1 1 1 1 0 0
 1 1 1 1 1 1 1 1 0 0
 0 0 0 1 1 1 1 1 0 0
 0 0 0 1 1 1 1 1 0 0
 '
  []
[]

#  0 0 0 0 0 0 0 0 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 0 0 0 1 1 1 1 0 0
#  1 0 1 0 1 1 1 1 0 0
#  1 1 1 0 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0

# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 0 0 0 1 1 1 1 1 0 0
# 0 0 0 1 1 1 1 1 0 0

[Problem]
  type = FEProblem
[]

[Variables]
  [temp]
    family = LAGRANGE
    order = FIRST
    scaling = 1e-3
  []
[]

[AuxVariables]
  [heat]
    family = MONOMIAL
    order = FIRST
  []

  [sink_var]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
    diffusion_coefficient = 'thermal_conductivity'
  []

  # [temp_source]
  #   type = HeatSourceBPD
  #   variable = temp
  #   power_density = heat
  #   # source_variable = heat
  #   # block = '0'
  # []

  [heat_source]
    type = CoupledForce
    variable = temp
    v = heat
    block = '1'
  []

  [sink]
    type = CoupledForce
    variable = temp
    v = sink_var
    block = '0'
  []

  # [temp_conduction]
  #   type = ADHeatConduction
  #   variable = temp
  #   thermal_conductivity = 'k'
  #   # block = '0'
  # []

  # [temp_source]
  #   type = INSADEnergySource
  #   variable = temp
  #   source_variable = heat
  #   # block = '0'
  # []

  # [temp_sink]
  #   type = INSADEnergyAmbientConvection
  #   variable = temp
  #   # alpha is the heat transfer coefficient.
  #   alpha = ${gamma}
  #   T_ambient = 900
  #   block = '1'
  # []

[]

[AuxKernels]
  [sink_aux]
    type = ParsedAux
    variable = sink_var
    coupled_variables = 'temp'
    expression = '-${sink_htc} * (temp - ${sink_temperature})'
    block = 0
  []
[]

# [BCs]

# []

[Materials]

  [block_0]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '${k_m}'
    block = 0
  []

  [block_1]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '${k_f}'
    block = 1
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_tol = 1e-05
[]

# [Preconditioning]
#   [SMP]
#     type = SMP
#     full = true
#   []
# []

[Postprocessors]

  [max_temperature]
    type = ElementExtremeValue
    value_type = max
    variable = temp
  []

  [bnorm_th]
    type = Receiver
  []

  [cost_function]
    type = ParsedPostprocessor
    pp_names = 'bnorm_th max_temperature'
    function = '-bnorm_th + 2.4e-06 * max_temperature'
    # function = '-bnorm_th'
    execute_on = TIMESTEP_END
  []
[]

[MultiApps]
  [ntsApp]
    type = FullSolveMultiApp
    app_type = MoltresApp
    execute_on = LINEAR
    # positions = '0 0 0'
    input_files = 'pin_nts.i'
    ignore_solve_not_converge = true
  []
[]

[Transfers]
  [to_sub_temp]
    type = MultiAppProjectionTransfer
    #     direction = to_multiapp
    #     multi_app = ntsApp
    to_multi_app = ntsApp
    source_variable = temp
    variable = temp
  []

  [from_sub]
    type = MultiAppProjectionTransfer
    #     direction = from_multiapp
    #     multi_app = ntsApp
    from_multi_app = ntsApp
    source_variable = heat
    variable = heat
  []

  [mesh_transfer]
    type = MultiAppMeshTransfer
    to_multi_app = ntsApp
    # from_multi_app = forward1
    execute_on = 'TIMESTEP_BEGIN'
  []

  [pp_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ntsApp
    reduction_type = maximum
    from_postprocessor = bnorm
    to_postprocessor = bnorm_th
  []

[]

[Outputs]
  [exodus]
    type = Exodus
    execute_on = 'Linear'
  []
[]

[Debug]
  # show_var_residual_norms = true
[]
