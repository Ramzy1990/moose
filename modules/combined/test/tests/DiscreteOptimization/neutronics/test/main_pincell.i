[Optimization]
[]

[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
  []
[]

[Reporters]
  [discrete_reporter]
    type = DiscreteOptimizationReporter
    allowed_mateirals = '0 1'
    excluded_materials = ' '
    outputs = "none"
    # pp = objective
    execute_on = "ALWAYS"
    #execute_on = 'FORWARD'
  []
[]

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  #solve_on = 'FORWARD'
  combinatorial_optimization = 1
  quarter_symmetry = 1
  check_density = 0 # Does not work with non-combinatorial (not easy to change solution). Mandatory for combinatorial for square?
  check_enclaves = 1
  check_boundaries = 1
  number_of_runs = 100
  number_of_iterations = 30
  debug_on = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = pin_cell_quarter.i
    execute_on = 'FORWARD'
  []
[]

[Transfers]
  [toforward]
    # check_multiapp_execute_on = false
    type = DiscreteOptimizationTransfer
    to_multi_app = forward
    user_object = 'discrete_reporter'
    debug = 0
    objective_name = 'cost_function'
  []
  [fromforward]
    # check_multiapp_execute_on = false
    type = DiscreteOptimizationTransfer
    from_multi_app = forward
    user_object = 'discrete_reporter'
    debug = 0
    objective_name = 'cost_function'
    execute_on = 'initial forward'
  []
[]

[Outputs]
  print_linear_residuals = false
  console = true
  execute_on = 'TIMESTEP_END FINAL'
  csv = true
[]
