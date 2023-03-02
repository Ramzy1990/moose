[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[MultiApps]
  [change]
    type = FullSolveMultiApp
    input_files = 'change_subdomain_assign.i'
  []
[]

[Transfers]
  [do_change]
    type = ChangeSubdomainTransfer
    to_multi_app = change
    user_object = change_subdomains
  []
  [do_change]

  []

[]

[Executioner]
  type = Steady
[]
