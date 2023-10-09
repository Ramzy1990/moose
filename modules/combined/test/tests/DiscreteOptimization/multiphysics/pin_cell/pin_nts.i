[GlobalParams]
  num_groups = 2
  group_fluxes = ' group1  group2'
  num_precursor_groups = 6
  use_exp_form = false
  sss2_input = true
  account_delayed = false
  create_temperature_var = false

  temperature = temp
  base_file = 'PinXS.json'
  # temperature = 600
  # base_file = 'PinXS_F_M.json'

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

[Problem]
  type = FEProblem
[]

[Nt]
  var_name_base = group
  # vacuum_boundaries = 'bottom left right top'
  #   transient = false
  eigen = true
  scaling = 1e3
[]

[AuxVariables]
  [temp]
    family = LAGRANGE
    order = FIRST
    initial_condition = 1000
  []
  [heat]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [heat_source]
    type = FissionHeatSourceAux
    variable = heat
    tot_fission_heat = powernorm
    power = 1e3
  []
[]

# [Materials]
#   [fuel]
#     type = GenericMoltresMaterial
#     # property_tables_root = './cnrs-benchmark/groups_2/benchmark_Fuel_'
#     property_tables_root = './cnrs-benchmark/benchmark_'
#     interp_type = 'linear'
#     block = '1'
#   []
#   # [moderator]
#   #   type = GenericMoltresMaterial
#   #   # property_tables_root = './cnrs-benchmark/groups_2/benchmark_Mod_'
#   #   # property_tables_root = './cnrs-benchmark/groups_6/benchmark_'
#   #   property_tables_root = './cnrs-benchmark/benchmark_'
#   #   interp_type = 'linear'
#   #   block = '1'
#   # []
# []

[Materials]
  [F]
    type = MoltresJsonMaterial
    block = '1'
    material_key = 'F'

    interp_type = 'linear'
    temperature = temp
    # interp_type = 'NONE'
    # temperature = 600

    prop_names = ''
    prop_values = ''
  []
  [W]
    type = MoltresJsonMaterial
    block = 0
    material_key = 'W'

    interp_type = 'linear'
    temperature = temp
    # interp_type = 'NONE'
    # temperature = 600

    prop_names = ''
    prop_values = ''
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

  [perimeter_fuel]
    type = RegionInterfaceAreaPostprocessor
    primary_block_names = '1'
    paired_block_names = '0'
    execute_on = TIMESTEP_END
  []

  [perimeter_mod]
    type = RegionInterfaceAreaPostprocessor
    primary_block_names = '0'
    paired_block_names = '1'
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
