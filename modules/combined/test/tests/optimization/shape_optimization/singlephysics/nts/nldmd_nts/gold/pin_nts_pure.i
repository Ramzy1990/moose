# Needs the latest version of Moltres, where they are using the Eigenvalue executioner instead of the Inverse power method executioner
# Finally, you need to add "use diblaced mesh" parameter to the NtAction of Moltres. A PR will be submitted to the Moltres team but it is straightforward to add

[GlobalParams]
  num_groups = 2
  group_fluxes = ' group1  group2'
  num_precursor_groups = 6
  use_exp_form = false
  sss2_input = true
  account_delayed = false
  create_temperature_var = false
  displacements = 'disp_x disp_y' # Needed for displacements to work, do not forget to add the AuxVariables for disp_x and disp_y and the use_displaced_mesh = true.
  temperature = 600
  base_file = 'pin_openmc_annular.json'
[]

[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 24
    radii = '0.2 0.54'
    rings = '7 7 7'
    has_outer_square = on
    pitch = 1.26
    preserve_volumes = on
    smoothing_max_it = 3
  []

  [inner]
    type = SideSetsBetweenSubdomainsGenerator
    input = ccmg
    new_boundary = inner
    primary_block = 1
    paired_block = 2
  []

  [outer]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner
    new_boundary = outer
    primary_block = 2
    paired_block = 3
  []

  [rename_blocks]
    type = RenameBlockGenerator
    old_block = '1               2    3'
    new_block = 'moderator_inner fuel moderator'
    input = outer
  []

[]

#[Debug]
#  show_material_props = true # An action in MOOSE
#[]

[Problem]
  type = EigenProblem
  bx_norm = fiss_neutrons
[]

[Nt]
  var_name_base = group
  # vacuum_boundaries = 'bottom left right top'
  #   transient = false
  eigen = true
  scaling = 1e-5
  use_displaced_mesh = true
[]

[AuxVariables]
  [heat]
    family = MONOMIAL
    order = FIRST
  []

  [disp_x]
  []

  [disp_y]
  []
[]

[AuxKernels]
  [heat_source]
    type = FissionHeatSourceAux
    variable = heat
    tot_fission_heat = powernorm
    power = 500
    use_displaced_mesh = true # we use displaced mesh here.
  []
[]

[MultiApps]
  [move_nodes]
    type = FullSolveMultiApp
    input_files = 'move_nodes.i'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [to_forward_Rinner]
    type = MultiAppReporterTransfer
    to_multi_app = move_nodes
    from_reporters = 'Ri/value'
    to_reporters = 'Ri/value'
  []

  [to_forward_Router]
    type = MultiAppReporterTransfer
    to_multi_app = move_nodes
    from_reporters = 'Ro/value'
    to_reporters = 'Ro/value'
  []

  [disp_x]
    type = MultiAppCopyTransfer
    from_multi_app = move_nodes
    source_variable = disp_x
    variable = disp_x
  []

  [disp_y]
    type = MultiAppCopyTransfer
    from_multi_app = move_nodes
    source_variable = disp_y
    variable = disp_y
  []
[]

[Reporters]
  [Ri]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0.0' #'-4.06396484375'
  []

  [Ro]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0.0' #'-3.9306640625'
  []
[]

[Postprocessors]
  [k_eff]
    type = VectorPostprocessorComponent
    index = 0
    vectorpostprocessor = k_vpp
    vector_name = eigen_values_real
    execute_on = timestep_end
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [fiss_neutrons]
    type = ElmIntegTotFissNtsPostprocessor
    block = '2'
    execute_on = linear
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [powernorm]
    type = ElmIntegTotFissHeatPostprocessor
    execute_on = linear
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [tot_fissions]
    type = ElmIntegTotFissPostprocessor
    execute_on = linear
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group1norm]
    type = ElementIntegralVariablePostprocessor
    variable = group1
    execute_on = linear
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group1max]
    type = NodalExtremeValue
    value_type = max
    variable = group1
    execute_on = timestep_end
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group1diff]
    type = ElementL2Diff
    variable = group1
    execute_on = 'linear timestep_end'
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group2norm]
    type = ElementIntegralVariablePostprocessor
    variable = group2
    execute_on = linear
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group2max]
    type = NodalExtremeValue
    value_type = max
    variable = group2
    execute_on = timestep_end
    use_displaced_mesh = true # we use displaced mesh here.
  []
  [group2diff]
    type = ElementL2Diff
    variable = group2
    execute_on = 'linear timestep_end'
    use_displaced_mesh = true # we use displaced mesh here.
  []

  [volume]
    type = VolumePostprocessor
    use_displaced_mesh = true # we use displaced mesh here.
    block = '2'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [objective]
    type = ParsedPostprocessor
    pp_names = 'k_eff'
    function = '1/k_eff'
  []
[]

[VectorPostprocessors]
  [k_vpp]
    type = Eigenvalues
    inverse_eigenvalue = true
  []
[]

[Materials]
  [W_in]
    type = MoltresJsonMaterial
    block = '1'
    material_key = 'coolant_in'
    interp_type = 'linear'
    temperature = 600
    # interp_type = 'NONE'
    # temperature = 600
    prop_names = ''
    prop_values = ''
  []
  [F]
    type = MoltresJsonMaterial
    block = '2'
    material_key = 'fuel'
    interp_type = 'linear'
    temperature = 600
    prop_names = ''
    prop_values = ''
  []
  [W]
    type = MoltresJsonMaterial
    block = '3'
    material_key = 'coolant'
    interp_type = 'linear'
    temperature = 600
    prop_names = ''
    prop_values = ''
  []
[]

[Executioner]
  type = Eigenvalue
  initial_eigenvalue = 1.0
  l_max_its = 100
  eigen_tol = 1e-7 # Increase as required
  free_power_iterations = 7
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-8 # Increase as required
  nl_abs_tol = 1e-8 # Increase as required
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = none
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Outputs]
  [exodus]
    type = ExodusOptimizationEigenvalue
    execute_on = 'TIMESTEP_END'
  []
[]
