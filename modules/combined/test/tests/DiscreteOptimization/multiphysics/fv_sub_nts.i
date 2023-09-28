[GlobalParams]
  num_groups = 6
  num_precursor_groups = 8
  use_exp_form = false
  group_fluxes = 'group1 group2 group3 group4 group5 group6'
  pre_concs = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
  temperature = temp
  sss2_input = true
  account_delayed = true
[]

# [Mesh]
#   type = GeneratedMesh
#   dim = 2

#   #   nx = 200
#   #   ny = 200
#   ## Use a 40-by-40 mesh instead if running on a desktop/small cluster
#   nx = 10
#   ny = 10

#   xmin = 0
#   xmax = 100
#   ymin = 0
#   ymax = 100
#   elem_type = QUAD4
# []

[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    ix = '1 1 1 1 1'
    iy = '1 1 1 1 1'
    # ix = '2 2 2 2 2 2 2 2 2 2'
    # iy = '2 2 2 2 2 2 2 2 2 2'
    # ix = '4 4 4 4 4 4 4 4 4 4'
    # iy = '4 4 4 4 4 4 4 4 4 4'
    dx = '20 20 20 20 20'
    dy = '20 20 20 20 20'
    # elem_type = QUAD4
    subdomain_id = '
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
'
  []
[]

[Problem]
  type = FEProblem
[]

[Nt]
  var_name_base = group
  # vacuum_boundaries = 'bottom left right top'
  vacuum_boundaries = 'right top'
  create_temperature_var = false
  #   transient = false
  eigen = true
  scaling = 1e3
[]

[Precursors]
  [pres]
    var_name_base = pre
    outlet_boundaries = 'right'
    constant_velocity_values = false
    uvel = velocity_x
    vvel = velocity_y
    nt_exp_form = false
    family = MONOMIAL
    order = CONSTANT
    loop_precursors = false
    transient = false
    eigen = true
    scaling = 1e3
  []
[]

[AuxVariables]
  [temp]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 900
  []
  [velocity_x]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [velocity_y]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [heat]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[AuxKernels]
  [heat_source]
    type = FissionHeatSourceAux
    variable = heat
    tot_fission_heat = powernorm
    power = 1e7
  []
[]

[Materials]
  # [fuel]
  #   type = GenericMoltresMaterial
  #   # property_tables_root = './cnrs-benchmark/groups_2/benchmark_Fuel_'
  #   property_tables_root = './cnrs-benchmark/benchmark_'
  #   interp_type = 'linear'
  #   block = '0'
  # []
  [moderator]
    type = GenericMoltresMaterial
    # property_tables_root = './cnrs-benchmark/groups_2/benchmark_Mod_'
    # property_tables_root = './cnrs-benchmark/groups_6/benchmark_'
    property_tables_root = './cnrs-benchmark/benchmark_'
    interp_type = 'linear'
    block = '1'
  []
[]

[Executioner]
  type = InversePowerMethod
  max_power_iterations = 50

  # fission power normalization
  # 1e7 corresponds to 1 GW / 100cm
  # Change to 2e6, 4e6, 6e6 or 8e6 for different power output
  # at 0.2GW, 0.4GW, 0.6GW or 0.8GW
  # normalization = 'powernorm' # This makes bnorm or k-eff computed correctly at the end of the simulation.
  # normal_factor = 1e7 # Watts, 1e9 / 100

  xdiff = 'group1diff'
  bx_norm = 'bnorm'
  k0 = 1.00400
  l_max_its = 100
  eig_check_tol = 1e-8
  # error_on_dtmin = false
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  #   petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_gasm_overlap -sub_pc_factor_shift_type -pc_gasm_blocks -sub_pc_factor_mat_solver_type'
  #   petsc_options_value = 'gasm     lu           200                1                NONZERO                   16              superlu_dist'

  ## Use the settings below instead if running on a desktop/small cluster
  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      lu           200                1               NONZERO'

  line_search = none
[]

# [BCs]
#   [reflective_1]
#     type = NeumannBC
#     variable = 'group1'
#     boundary = 'bottom left'
#     value = 0
#   []
#   [reflective_2]
#     type = NeumannBC
#     variable = 'group2'
#     boundary = 'bottom left'
#     value = 0
#   []
#   [reflective_3]
#     type = NeumannBC
#     variable = 'group3'
#     boundary = 'bottom left'
#     value = 0
#   []
#   [reflective_4]
#     type = NeumannBC
#     variable = 'group4'
#     boundary = 'bottom left'
#     value = 0
#   []
#   [reflective_5]
#     type = NeumannBC
#     variable = 'group5'
#     boundary = 'bottom left'
#     value = 0
#   []
#   [reflective_6]
#     type = NeumannBC
#     variable = 'group6'
#     boundary = 'bottom left'
#     value = 0
#   []
# []

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
  [powernorm]
    type = ElmIntegTotFissHeatPostprocessor
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
[]

[VectorPostprocessors]
  [pre_elemental]
    type = ElementValueSampler
    variable = 'pre1 pre2 pre3 pre4 pre5 pre6 pre7 pre8'
    sort_by = id
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  # perf_graph = true
  # print_linear_residuals = true
  [exodus]
    type = Exodus
  []
  # [csv]
  #   type = CSV
  # []
[]

[Debug]
  # show_var_residual_norms = true
[]
