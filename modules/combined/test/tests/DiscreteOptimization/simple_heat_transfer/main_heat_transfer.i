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
    allowed_mateirals = '1 2'
    excluded_materials = ' '
    outputs = "none"
  []
[]

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  # constraints_user_object = discrete_constraints
  solve_on = 'FORWARD'
  combinatorial_optimization = 0
  # dimension = 2
  quarter_symmetry = 0
  check_density = 0
  check_enclaves = 0
  check_boundaries = 0
  number_of_runs = 10000
  number_of_iterations = 10
  maximum_temperature = 1000
  minimum_temperature = 0.001
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = simple_heat_cool.i
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [toforward]
    type = DiscreteOptimizationTransfer
    to_multi_app = forward
    user_object = 'discrete_reporter'
    debug = 0
    objective_name = 'cost_function'
  []
  [fromforward]
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
  csv = true
[]

