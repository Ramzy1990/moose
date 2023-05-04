[Optimization]
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1 1 1 1 1 1 1 1 1 1'
    dy = '1 1 1 1 1 1 1 1 1 1'
    dz = '1 1 1 1 1 1 1 1 1 1'
    # subdomain_id = '1 2 3'
  []
[]

[OptimizationReporter]
  type = DiscreteOptimizationReporter
  parameter_names = Materials
  # Number of parameter names we are adjusting for. since we have materials
  # only, so it will be 1. Or it can be the number of materials we are adjusting for?
  num_values = 1
  initial_material = '0'
  assign_type = 'auto'
  solver_type = 'random_3'
  number_of_elements = 12
  allowed_mateirals = '0 1 2'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
[]
