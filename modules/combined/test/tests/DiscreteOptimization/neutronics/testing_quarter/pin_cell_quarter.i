# halfa = 10
# fulla = 10

global_temperature = 600

[GlobalParams]
  num_groups = 2
  base_file = 'PinXS_F_M.json'
  group_fluxes = ' group1  group2'
  num_precursor_groups = 6
  use_exp_form = false
  temperature = ${global_temperature}
[]

# [Mesh]
# #   file = pin_cell_mesh.e
#   [cmg]
#     type = CartesianMeshGenerator
#     # elem_type = Tri3
#     dim = 2
#     dx = '1 1 1 1 1 1 1 1 1'
#     dy = '1 1 1 1 1 1 1 1 1'
# #     ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
# #     iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
#     subdomain_id = '1 1 1 1 1 1 1 1 2
#                     1 1 1 1 1 1 1 1 2
#                     1 1 1 1 1 1 1 1 2
#                     1 1 1 1 1 1 1 2 2
#                     1 1 1 1 1 1 1 2 2
#                     1 1 1 1 1 1 2 2 2
#                     1 1 1 1 1 2 2 2 2
#                     1 1 1 2 2 2 2 2 2
#                     2 2 2 2 2 2 2 2 2'
#   []
# []

# [Mesh]
#   #   file = pin_cell_mesh.e
#   [cmg]
#     type = CartesianMeshGenerator
#     # elem_type = Tri3
#     dim = 2
#     dx = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
#     dy = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
#     #     ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
#     #     iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
#     subdomain_id = '

# 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
# 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 0 0 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 0 0 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
# 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0
# 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
#   []
# []

# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1

[Mesh]
  #   file = pin_cell_mesh.e
  [cmg]
    type = CartesianMeshGenerator
    # elem_type = Tri3
    dim = 2
    dx = '0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126'
    dy = '0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126'
    # dz = '0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126 0.126'
    # dz = '0.126 0.126 0.126 0.126 0.126'
    #     ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    #     iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    subdomain_id = '

    0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0
    1 1 1 1 1 1 0 0 0 0
    1 1 1 1 1 1 0 1 0 0
    1 1 1 1 1 1 1 1 0 0
    1 1 1 1 1 1 1 1 0 0
    1 1 1 1 1 1 1 1 0 0
    1 1 1 1 1 1 1 1 0 0
    0 1 1 1 1 1 1 1 0 0
    0 0 1 1 1 1 1 1 0 0
    '

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 1 1 1 1 1 1 0 0 0 0
    # 1 1 1 1 1 1 0 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 0 1 1 1 1 1 1 1 0 0
    # 0 0 1 1 1 1 1 1 0 0

  []
[]

[Problem]
  type = FEProblem
  #   coord_type = XYZ
[]

[Nt]
  var_name_base = group
  create_temperature_var = false
  eigen = true
  account_delayed = 0
  sss2_input = 1
[]

[Materials]
  [F]
    type = MoltresJsonMaterial
    block = '1'
    material_key = 'F'
    interp_type = 'NONE'
    temperature = 600
    prop_names = ''
    prop_values = ''
  []
  [W]
    type = MoltresJsonMaterial
    block = 0
    material_key = 'W'
    interp_type = 'NONE'
    temperature = 600
    prop_names = ''
    prop_values = ''
  []
[]

[Executioner]
  # type = NonlinearEigen
  type = InversePowerMethod
  nl_abs_tol = 1e-6
  # free_power_iterations = 10
  Chebyshev_acceleration_on = false
  bx_norm = 'bnorm'
  k0 = 1.0
  l_max_its = 100
  # line_search = none
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  # petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  # petsc_options_iname = '-pc_type -sub_pc_type'
  # petsc_options_value = 'asm lu'
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [bnorm]
    type = ElmIntegTotFissNtsPostprocessor
    execute_on = linear
  []
  [tot_fissions]
    type = ElmIntegTotFissPostprocessor
    execute_on = linear
  []
  [group1norm]
    type = ElementIntegralVariablePostprocessor
    variable = group1
    execute_on = linear
  []
  [group1max]
    type = NodalExtremeValue
    value_type = max
    variable = group1
    execute_on = timestep_end
  []
  [group1diff]
    type = ElementL2Diff
    variable = group1
    execute_on = 'linear timestep_end'
    use_displaced_mesh = false
  []
  [group2norm]
    type = ElementIntegralVariablePostprocessor
    variable = group2
    execute_on = linear
  []
  [group2max]
    type = NodalExtremeValue
    value_type = max
    variable = group2
    execute_on = timestep_end
  []
  [group2diff]
    type = ElementL2Diff
    variable = group2
    execute_on = 'linear timestep_end'
    use_displaced_mesh = false
  []

  [perimeter_fuel]
    type = RegionInterfaceAreaPostprocessor
    primary_block_names = '1'
    paired_block_names = '0'
    execute_on = TIMESTEP_END
  []

  # [area_all]
  #   type = RegionInterfaceAreaPostprocessor
  # []

  [area_all_moderator]
    type = VolumePostprocessor
    block = '0'
    execute_on = TIMESTEP_END
  []

  [area_all_fuel]
    type = VolumePostprocessor
    block = '1'
    execute_on = TIMESTEP_END
  []

  [area_all]
    type = VolumePostprocessor
    block = '0 1'
    execute_on = TIMESTEP_END
  []

  [hydraulic_diameter]
    type = ParsedPostprocessor
    # pp_names = 'perimeter_fuel area_all_fuel area_all_moderator'
    pp_names = 'perimeter_fuel area_all'
    # function = '-1*(bnorm * if(max_temperature < 360, 1, 0.01))'
    # function = '(area_all_fuel+area_all_moderator)/ perimeter_fuel'

    # This is how it should be after testing the individual input file with different
    # confgurations including water in the middle or tetris shape around the middle and seeing how cost function is behaving.
    function = '4 * area_all/ perimeter_fuel'

    # function = 'perimeter_fuel/area_all'
    # function = '-(total_source - (max_temperature / 2))'
    # function = 'max_temperature/total_source'
    execute_on = TIMESTEP_END
  []

  # [cost_function]
  #   # type = ElementL2Diff
  #   type = ElmIntegTotFissNtsPostprocessor
  #   execute_on = linear
  #   # value_type = max
  #   # variable = eigenvalue
  #   # execute_on = 'linear timestep_end'
  #   # use_displaced_mesh = false
  # []
  [cost_function]
    type = ParsedPostprocessor
    # pp_names = 'bnorm'
    pp_names = 'bnorm hydraulic_diameter'
    # function = '-1 * ( bnorm )'
    # function = '1/bnorm + 0.00001/(hydraulic_diameter*hydraulic_diameter*hydraulic_diameter*hydraulic_diameter)'
    function = '-bnorm - 0.00001 * hydraulic_diameter * hydraulic_diameter * hydraulic_diameter * hydraulic_diameter'
    # function = '-1 * bnorm + 0.0001 * hydraulic_diameter * hydraulic_diameter * hydraulic_diameter * hydraulic_diameter '
    # function = '-bnorm'
    execute_on = TIMESTEP_END
  []
[]

# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 1 1 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 0 0
# 0 0 1 1 1 1 1 1 0 0

# [BCs]

#   [top]
#     type = NeumannBC
#     variable = group1
#     boundary = top
#     value = 0
#   []

#   [left]
#     type = NeumannBC
#     variable = group1
#     boundary = left
#     value = 0
#   []

#   [bottom]
#     type = NeumannBC
#     variable = group1
#     boundary = bottom
#     value = 0
#   []

#   [right]
#     type = NeumannBC
#     variable = group1
#     boundary = right
#     value = 0
#   []

#   [left_group2]
#     type = NeumannBC
#     variable = group2
#     boundary = left
#     value = 0
#   []

#   [top_group2]
#     type = NeumannBC
#     variable = group2
#     boundary = top
#     value = 0
#   []

#   [bottom_group2]
#     type = NeumannBC
#     variable = group2
#     boundary = bottom
#     value = 0
#   []

#   [right_group2]
#     type = NeumannBC
#     variable = group2
#     boundary = right
#     value = 0
#   []

# []

[Outputs]
  [out]
    type = Exodus
    execute_on = 'TIMESTEP_END'
  []
[]
