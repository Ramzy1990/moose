[GlobalParams]
  # displacements = 'disp_x disp_y'
[]

[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 24
    radii = '0.2 0.5'
    rings = '3 6 5'
    has_outer_square = on
    pitch = 1.26
    preserve_volumes = on
    smoothing_max_it = 3
  []

  [rename_blocks]
    type = RenameBlockGenerator
    old_block = '1         2    3'
    new_block = 'moderator fuel moderator'
    input = ccmg
  []

  block_id = '1 2'
  block_name = 'moderator fuel'
[]

[Debug]
  show_material_props = true
[]

[TransportSystems]
  particle = neutron
  equation_type = eigenvalue
  G = 7
  ReflectingBoundary = 'right top left bottom'
  [diff]
    scheme = CFEM-Diffusion
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Materials]
  [fuel]
    type = ConstantNeutronicsMaterial
    block = fuel
    fromFile = true
    library_file = c5g7_materials.xml
    material_id = 1
    diffusion_coefficient_scheme = local
  []

  [moderator]
    type = ConstantNeutronicsMaterial
    block = moderator
    fromFile = true
    library_file = c5g7_materials.xml
    material_id = 7
    diffusion_coefficient_scheme = local
  []
[]

# [MultiApps]
#   [move_nodes]
#     type = FullSolveMultiApp
#     input_files = 'move_nodes.i'
#     execute_on = TIMESTEP_BEGIN
#   []
# []

# [Transfers]
#   [to_forward_Rinner]
#     type = MultiAppReporterTransfer
#     to_multi_app = move_nodes
#     from_reporters = 'Ri/value'
#     to_reporters = 'Ri/value'
#   []

#   [to_forward_Router]
#     type = MultiAppReporterTransfer
#     to_multi_app = move_nodes
#     from_reporters = 'Ro/value'
#     to_reporters = 'Ro/value'
#   []

#   [disp_x]
#     type = MultiAppCopyTransfer
#     from_multi_app = move_nodes
#     source_variable = disp_x
#     variable = disp_x
#   []

#   [disp_y]
#     type = MultiAppCopyTransfer
#     from_multi_app = move_nodes
#     source_variable = disp_y
#     variable = disp_y
#   []
# []

[Postprocessors]
  [volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
    block = 'fuel'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [cost_pp]
    type = ParsedPostprocessor
    pp_names = 'eigenvalue'
    function = '1 / eigenvalue'
  []
[]

[Reporters]
  [Ri]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0'
  []

  [Ro]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0'
  []
[]

[Executioner]
  type = Eigenvalue

  free_power_iterations = 1
  nl_abs_tol = 1e-6
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
[]

[Outputs]
  #console = false
  exodus = true
[]
