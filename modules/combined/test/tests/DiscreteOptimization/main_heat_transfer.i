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
    parameter_names = Materials
    # Number of parameter names we are adjusting for. since we have materials
    # only, so it will be 1. Or it can be the number of materials we are adjusting for?
    num_values = 1
    initial_material = '1'
    assign_type = 'auto'
    solver_type = 'random_3'
    number_of_elements = 12
    allowed_mateirals = '1 2'
    excluded_materials = ' '
    outputs = "none"
  []
[]

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  solve_on = 'FORWARD'
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

