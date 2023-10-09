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
  block_name = 'fuel moderator'
[]

[Debug]
  show_material_props = true
[]

[TransportSystems]
  particle = neutron
  equation_type = eigenvalue
  G = 7
  ReflectingBoundary = 'right top left bottom'
  [sn]
    scheme = DFEM-SN
    sweep_type = on_processor_sweep
    family = L2_LAGRANGE
    order = FIRST
    AQorder = 8
    AQtype = Level-Symmetric
    hide_angular_flux = true
    verbose = 0
    collapse_scattering = true
    using_array_variable = true
  []
[]

[Preconditioning]
  [sweep]
    type = SNSweepPreconditioner
    show_cpu_time = true
    #inner_solve_type = GMRes
    max_inner_its = 3
  []
[]

[Materials]
  [fuel]
    type = ConstantNeutronicsMaterial
    block = fuel
    fromFile = true
    library_file = c5g7_materials.xml
    material_id = 1
  []

  [moderator]
    type = ConstantNeutronicsMaterial
    block = moderator
    fromFile = true
    library_file = c5g7_materials.xml
    material_id = 7
  []
[]

[Executioner]
  type = Eigenvalue

  free_power_iterations = 1
  nl_abs_tol = 1e-10
  solve_type = 'JFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  perf_graph = true
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
