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

[Mesh]
  #   file = pin_cell_mesh.e
  [cmg]
    type = CartesianMeshGenerator
    # elem_type = Tri3
    dim = 2
    dx = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
    dy = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
    #     ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    #     iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    subdomain_id = '

0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0
0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0
0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0
0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'

    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 1 0 0
    # 1 1 1 1 1 1 1 0 0 0
    # 0 0 0 0 0 0 0 0 0 0
    # 0 0 0 0 0 0 0 0 0 0'
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
  type = NonlinearEigen
  nl_abs_tol = 1e-6
  bx_norm = 'bnorm'
  k0 = 1.0
  l_max_its = 100

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
    pp_names = 'bnorm'
    # function = '-1*(bnorm * if(max_temperature < 360, 1, 0.01))'
    function = '-1*(bnorm)'
    # function = '-(total_source - (max_temperature / 2))'
    # function = 'max_temperature/total_source'
    execute_on = linear
  []

[]

# [BCs]

#   [left]
#     type = DirichletBC
#     variable = group1
#     boundary = left
#     value = 0
#   []

#   [bottom]
#     type = DirichletBC
#     variable = group1
#     boundary = bottom
#     value = 0
#   []

#   [top]
#     type = DirichletBC
#     variable = group1
#     boundary = top
#     value = 0
#   []

#   [right]
#     type = DirichletBC
#     variable = group1
#     boundary = right
#     value = 0
#   []

#   [left_group2]
#     type = DirichletBC
#     variable = group2
#     boundary = left
#     value = 0
#   []

#   [bottom_group2]
#     type = DirichletBC
#     variable = group2
#     boundary = bottom
#     value = 0
#   []

#   [top_group2]
#     type = DirichletBC
#     variable = group2
#     boundary = top
#     value = 0
#   []

#   [right_group2]
#     type = DirichletBC
#     variable = group2
#     boundary = right
#     value = 0
#   []

# []

[Outputs]
  [out]
    type = Exodus
    execute_on = 'timestep_end'
  []
[]
