# density_f = 2.146e-3 # kg cm-3
k_f = 0.105 # W cm-1 K-1
k_m = 0.00600 # W cm-1 K-1
# sink_htc = 100
# sink_temperature = 600

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
    # ix = '4 4 4 4 4 4 4 4 4 4'
    # iy = '4 4 4 4 4 4 4 4 4 4'
    subdomain_id = '

0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0
1 1 1 1 1 0 0 0 0 0
1 1 1 1 1 1 0 0 0 0
1 1 1 1 1 1 1 0 0 0
1 1 1 1 1 1 1 1 0 0
1 1 1 1 1 1 1 1 0 0
1 1 1 1 1 1 1 1 0 0
1 1 1 1 1 1 1 1 0 0
1 1 1 1 1 1 1 1 0 0

 '
  []

  [add_boundary_all]
    type = ParsedGenerateSideset
    combinatorial_geometry = '2 > 1'
    input = cmg
    new_sideset_name = all_sides
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

#  0 0 0 0 0 0 0 0 0 0
#  0 0 0 0 0 0 0 0 0 0
#  1 1 1 1 1 0 0 0 0 0
#  1 1 1 1 1 1 0 0 0 0
#  1 1 1 1 1 1 1 0 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0
#  1 1 1 1 1 1 1 1 0 0

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

  # [sink_var]
  #   family = MONOMIAL
  #   order = CONSTANT
  # []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
    diffusion_coefficient = 'thermal_conductivity'
  []

  [heat_source]
    type = CoupledForce
    variable = temp
    v = heat
    block = '1'
  []

  # [sink]
  #   type = CoupledForce
  #   variable = temp
  #   v = sink_var
  #   block = '0'
  # []
[]

[AuxKernels]
  # [sink_aux]
  #   type = ParsedAux
  #   variable = sink_var
  #   coupled_variables = 'temp'
  #   expression = '-${sink_htc} * (temp - ${sink_temperature})'
  #   block = 0
  # []
[]

[BCs]
  [convective]
    type = CoupledConvectiveHeatFluxBC_Mod
    variable = temp
    htc = 1000
    T_infinity = 600
    boundary = all_sides
  []
[]

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
    execute_on = 'Linear TIMESTEP_END'
  []

  [bnorm_th]
    type = Receiver
  []

  [elem_90]
    type = ElementalVariableValue
    variable = heat
    elementid = 90
  []

  [cost_function]
    type = ParsedPostprocessor
    pp_names = 'bnorm_th elem_90 '
    function = '-1*(bnorm_th * if(elem_90 < 100, 1, 0.01))'
    # function = '-1*(bnorm_th * if(elem_90 = 0.000e+00, 1, 0.1))'
    # function = '(-bnorm_th + elem_90) * if(elem_90 = 0.0, 1, 0.0001)'
    # function = '-bnorm_th + 2.4e-06 * max_temperature'
    # function = '-bnorm_th'
    execute_on = 'TIMESTEP_END'
  []

  [elem_91]
    type = ElementalVariableValue
    variable = heat
    elementid = 91
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
    execute_on = 'TIMESTEP_BEGIN LINEAR TIMESTEP_END'
  []

  [from_sub]
    type = MultiAppProjectionTransfer
    #     direction = from_multiapp
    #     multi_app = ntsApp
    from_multi_app = ntsApp
    source_variable = heat
    variable = heat
    execute_on = 'TIMESTEP_BEGIN LINEAR TIMESTEP_END'
  []

  [mesh_transfer]
    type = MultiAppMeshTransfer
    to_multi_app = ntsApp
    # from_multi_app = forward1
    # execute_on = 'TIMESTEP_BEGIN'
    execute_on = 'TIMESTEP_BEGIN LINEAR TIMESTEP_END'
  []

  [pp_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ntsApp
    reduction_type = maximum
    from_postprocessor = bnorm
    to_postprocessor = bnorm_th
    execute_on = 'TIMESTEP_BEGIN LINEAR TIMESTEP_END'
  []

[]

[Outputs]
  [exodus]
    type = Exodus
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END LINEAR FINAL'
    overwrite = true
  []
[]

[Debug]
  # show_var_residual_norms = true
[]
