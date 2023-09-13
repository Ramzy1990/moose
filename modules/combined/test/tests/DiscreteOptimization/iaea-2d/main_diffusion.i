# Note that the output will be a 17 x 17 mesh

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
    allowed_mateirals = '1 2 3 4'
    excluded_materials = '3 4'
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
  number_of_runs = 100
  number_of_iterations = 10
  maximum_temperature = 100
  minimum_temperature = 0.01
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    # input_files = diffusion_reaction.i
    input_files = diffusion_reaction2.i
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
