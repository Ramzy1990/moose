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
    # execute_on = "ALWAYS"
  []
[]

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  # constraints_user_object = discrete_constraints
  solve_on = 'FORWARD'
  combinatorial_optimization = 1
  # dimension = 2
  quarter_symmetry = 0
  check_density = 0
  check_enclaves = 0
  check_boundaries = 0
  number_of_runs = 100
  number_of_iterations = 20
  debug_on = 1
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = sub_th.i
    execute_on = 'TIMESTEP_BEGIN'
    # app_type = GriffinApp
    ignore_solve_not_converge = true
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
  []
[]

[Outputs]
  print_linear_residuals = false
  console = true
  # exodus = true
  csv = true
  # [console]
  # type = Console
  # max_rows = 1
  # []
[]
