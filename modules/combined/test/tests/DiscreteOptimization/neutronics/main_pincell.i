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
    allowed_mateirals = '0 1'
    excluded_materials = ' '
    outputs = "none"
    # pp = objective
    # execute_on = "ALWAYS"
  []
  #   [test123]
  #     type = DiscreteOptimizationReporter
  #     outputs = "none"
  #   []
[]

[Executioner]
  type = CustomOptimize
  reporter_user_object = discrete_reporter
  solve_on = 'FORWARD'
  # type = gardensnake-opt
  # nl_max_its = 50
  # execute_on = 'TIMESTEP_END'
  # num_steps = 500
  # dt = 0.1
  # solve_type = PJFNK
  # nl_abs_tol = 1e-6
  # nl_rel_tol = 1e-8
  # petsc_options_iname = '-pc_type'
  # petsc_options_value = 'lu'
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
    input_files = pin_cell.i
    # input_files = diffusion_reaction.i
    # input_files = diffusion_reaction2.i
    # input_files = simple_heat_cool.i
    # input_files = eigen_test.i
    execute_on = 'TIMESTEP_BEGIN'
    # execute_on = 'FORWARD'
    #reset_apps = '0 0'
    #reset_time = '1 2'
    #execute_on = TIMESTEP_END
    # execute_on = 'INITIAL TIMESTEP_BEGIN'
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
  csv = true
  # [console]
  # type = Console
  # max_rows = 1
  # []
[]

# [UserObjects]
#   [discrete_reporter_object]
#     type = DiscreteOptimizationReporter
#   []
# []

