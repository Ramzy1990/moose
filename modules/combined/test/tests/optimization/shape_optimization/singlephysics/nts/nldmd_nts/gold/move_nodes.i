[GlobalParams]
  displacements = 'disp_x disp_y'
  order = FIRST
[]

[Problem]
  type = FEProblem
[]

[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 24
    radii = '0.200 0.54'
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

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [disp_x]
    type = MatDiffusion
    variable = disp_x
    use_displaced_mesh = false
    diffusivity = 1 #diff_coef
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
    use_displaced_mesh = false
    diffusivity = 1 #diff_coef
  []
[]

[Functions]
  [r1_x]
    type = ParsedOptimizationFunction
    expression = 'r * cos((atan(y/x))) / 100'
    param_symbol_names = 'r'
    param_vector_name = 'Ri/value'
  []

  [r1_y]
    type = ParsedOptimizationFunction
    expression = 'r * sin((atan(y/x))) / 100'
    param_symbol_names = 'r'
    param_vector_name = 'Ri/value'
  []

  [r0_x]
    type = ParsedOptimizationFunction
    expression = 'r * cos((atan(y/x))) / 100'
    param_symbol_names = 'r'
    param_vector_name = 'Ro/value'
  []

  [r0_y]
    type = ParsedOptimizationFunction
    expression = 'r * sin((atan(y/x))) / 100'
    param_symbol_names = 'r'
    param_vector_name = 'Ro/value'
  []
[]

[BCs]
  [pin_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left right bottom top'
    value = 0
  []

  [pin_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'left right bottom top'
    value = 0
  []

  [diffuse_r1_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 'outer'
    function = r1_x
    preset = false
  []
  [diffuse_r1_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'outer'
    function = r1_y
    preset = false
  []
  [diffuse_r0_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 'inner'
    function = r0_x
    preset = false
  []
  [diffuse_r0_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'inner'
    function = r0_y
    preset = false
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
    block = '2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Reporters]
  [Ri]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0.0'
  []

  [Ro]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '0.0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
  nl_rel_tol = 1e-10
  nl_max_its = 500
[]

[Outputs]
  #console = false
[]