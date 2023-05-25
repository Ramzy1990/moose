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
    outputs = "none"
    # execute_on = "ALWAYS"
  []
  #   [test123]
  #     type = DiscreteOptimizationReporter
  #     outputs = "none"
  #   []
[]

[Executioner]
  type = Transient
  num_steps = 10
  # dt = 0.1
  # solve_type = PJFNK
  # nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
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
    input_files = simple_heat_cool.i
    # input_files = 'simple_heat_cool.i'
    # execute_on = 'INITIAL TIMESTEP_BEGIN'
    # execute_on = 'FORWARD'
    #reset_apps = '0 0'
    #reset_time = '1 2'
    #execute_on = TIMESTEP_END
    # execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  print_linear_residuals = false
  console = true
  # exodus = true
  csv = true
[]

# [UserObjects]
#   [discrete_reporter_object]
#     type = DiscreteOptimizationReporter
#   []
# []

[Transfers]
  #active = ''
  [toforward]
    type = DiscreteOptimizationTransferTest
    to_multi_app = forward
    user_object = 'discrete_reporter'
  []
  [fromforward]
    type = DiscreteOptimizationTransferTest
    from_multi_app = forward
    user_object = 'discrete_reporter'
  []
[]

