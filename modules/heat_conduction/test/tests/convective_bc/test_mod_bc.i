[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    ix = '10 10 10 10'
    dy = '1 1 1 1'
    iy = '10 10 10 10'
    subdomain_id = '0 0 0 0
                        0 0 0 0
                        1 1 0 0
                        1 1 0 0'
  []

  [add_boundary_all]
    type = ParsedGenerateSideset
    combinatorial_geometry = '2 > 1'
    input = gen
    new_sideset_name = all_sides
  []
[]

[Variables/T]
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = 1
  []

  [src]
    type = HeatSource
    variable = T
    value = 10
    block = 1
  []

  [null]
    type = NullKernel
    variable = T
    block = 0
  []
[]

[BCs]
  [convective]
    type = CoupledConvectiveHeatFluxBC_Mod
    variable = T
    htc = 1
    T_infinity = 0
    boundary = all_sides
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 50'
[]

[Outputs]
  exodus = true
[]
