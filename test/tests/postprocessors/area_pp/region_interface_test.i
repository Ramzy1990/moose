[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    dy = '1 1 1'
    subdomain_id = '0 1 0
                    1 2 1
                    0 1 1'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [area0_1]
    type = RegionInterfaceAreaPostprocessor
    primary_block_names = '0'
    paired_block_names = '1'
  []

  [area02_1]
    type = RegionInterfaceAreaPostprocessor
    primary_block_names = '0 2'
    paired_block_names = '1'
  []

  [area_all]
    type = RegionInterfaceAreaPostprocessor
  []
[]

[Outputs]
  file_base = out
  exodus = true
[]
