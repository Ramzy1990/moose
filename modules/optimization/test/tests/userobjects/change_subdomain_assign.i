[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1 1'
    subdomain_id = '1 2 3'
  []
[]

[AuxVariables/u]
[]


[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [change_subdomains]
    type = ChangeSubdomainAssignment
    execute_on = INITIAL
  []
[]

[Outputs]
  exodus = true
[]