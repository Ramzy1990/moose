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

# [Cons]
#   [discrete_constraints]
#     type = DiscreteConstraintsLibrary
#     check_bounding_box_density = 1
#   []
# []

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  # constraints_user_object = discrete_constraints
  solve_on = 'FORWARD'
  combinatorial_optimization = 1
  # dimension = 2
  quarter_symmetry = 1
  check_density = 1 # Does not work with non-combinatorial (not easy to change solution). Mandatory for combinatorial for square?
  check_enclaves = 1
  check_boundaries = 1
  number_of_runs = 100
  number_of_iterations = 30
  debug_on = true
[]

# [Problem]
#   solve = false
#   # type = DiscreteOptimizationReporter
#   # outputs = none
#   # []
# []

# [Debug]
#   show_actions = true
#   show_action_dependencies = true
# []

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = pin_cell_quarter.i
    execute_on = 'TIMESTEP_BEGIN'
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
    # objective_name = 'max_eigenvalue'
  []
  [fromforward]
    # check_multiapp_execute_on = false
    type = DiscreteOptimizationTransfer
    from_multi_app = forward
    user_object = 'discrete_reporter'
    debug = 0
    objective_name = 'cost_function'
    # objective_name = 'max_eigenvalue'
  []
[]

[Outputs]
  print_linear_residuals = false
  console = true
  # exodus = true
  execute_on = 'TIMESTEP_END FINAL'
  csv = true
  # [console]
  # type = Console
  # max_rows = 1
  # []
[]

