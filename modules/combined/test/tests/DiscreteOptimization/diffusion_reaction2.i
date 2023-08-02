halfa = 1
fulla = 2

[Problem]
  type = FEProblem
  extra_tag_vectors = 'diff0 diff1 diff2 diff3 abs0 abs1 abs2 abs3 src0 src1 src2'
[]

[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 2 2 2 2 2 2 2 2'
    dy = '1 2 2 2 2 2 2 2 2'
    ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    subdomain_id = '2 1 1 1 2 1 1 3 4
                    1 1 1 1 1 1 1 3 4
                    1 1 2 1 1 1 3 3 4
                    1 1 1 1 1 1 3 4 4
                    2 1 1 1 2 3 3 4 4
                    1 1 1 1 3 3 4 4 4
                    1 1 3 3 3 4 4 4 4
                    3 3 3 4 4 4 4 4 4
                    4 4 4 4 4 4 4 4 4'
  []
[]

[Variables]
  [psi]
  []
[]

[Kernels]

  [diff0]
    type = MatDiffusion
    variable = psi
    diffusivity = D0
    extra_vector_tags = 'diff0'
    block = 1
  []
  [diff1]
    type = MatDiffusion
    variable = psi
    diffusivity = D1
    extra_vector_tags = 'diff1'
    block = 2
  []
  [diff2]
    type = MatDiffusion
    variable = psi
    diffusivity = D2
    extra_vector_tags = 'diff2'
    block = 3
  []
  [diff3]
    type = MatDiffusion
    variable = psi
    diffusivity = D3
    extra_vector_tags = 'diff3'
    block = 4
  []

  [abs0]
    type = MatReaction
    variable = psi
    mob_name = absxs0
    extra_vector_tags = 'abs0'
    block = 1
  []
  [abs1]
    type = MatReaction
    variable = psi
    mob_name = absxs1
    extra_vector_tags = 'abs1'
    block = 2
  []
  [abs2]
    type = MatReaction
    variable = psi
    mob_name = absxs2
    extra_vector_tags = 'abs2'
    block = 3
  []
  [abs3]
    type = MatReaction
    variable = psi
    mob_name = absxs3
    extra_vector_tags = 'abs3'
    block = 4
  []

  [src0]
    type = BodyForce
    variable = psi
    value = 20
    extra_vector_tags = 'src0'
    block = 1
  []
  [src1]
    type = BodyForce
    variable = psi
    value = 20
    extra_vector_tags = 'src1'
    block = 2
  []
  [src2]
    type = BodyForce
    variable = psi
    value = 20
    extra_vector_tags = 'src2'
    block = 3
  []

[]

[Materials]

  [D0]
    type = GenericConstantMaterial
    prop_names = 'D0'
    prop_values = 0.8
    block = 1
  []
  [D1]
    type = GenericConstantMaterial
    prop_names = 'D1'
    prop_values = 0.8
    block = 2
  []
  [D2]
    type = GenericConstantMaterial
    prop_names = 'D2'
    prop_values = 0.8
    block = 3
  []
  [D3]
    type = GenericConstantMaterial
    prop_names = 'D3'
    prop_values = 0.6
    block = 4
  []

  [absxs0]
    type = GenericConstantMaterial
    prop_names = 'absxs0'
    prop_values = -0.17
    block = 1
  []
  [absxs1]
    type = GenericConstantMaterial
    prop_names = 'absxs1'
    prop_values = -0.26
    block = 2
  []
  [absxs2]
    type = GenericConstantMaterial
    prop_names = 'absxs2'
    prop_values = -0.16
    block = 3
  []
  [absxs3]
    type = GenericConstantMaterial
    prop_names = 'absxs3'
    prop_values = -0.02
    block = 4
  []

[]

[BCs]

  [left]
    type = NeumannBC
    variable = psi
    boundary = left
    value = 0
  []

  [bottom]
    type = NeumannBC
    variable = psi
    boundary = bottom
    value = 0
  []

  [top]
    type = DirichletBC
    variable = psi
    boundary = top
    value = 0
  []

  [right]
    type = DirichletBC
    variable = psi
    boundary = right
    value = 0
  []

[]

[Executioner]
  type = Steady
  # type = Steady
  # type = Transient
  nl_abs_tol = 1e-8
  solve_type = PJFNK
  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

[]

# [Controls]
#   [stochastic]
#     type = SamplerReceiver
#   []
# []

[Postprocessors]

  [nodal_l2]
    type = NodalL2Norm
    variable = psi
  []

  [cost_function]
    type = ElementExtremeValue
    value_type = max
    variable = psi
  []

[]

[Outputs]
  print_linear_residuals = false
  exodus = true
  csv = true
  # [console]
  # type = Console
  # print_mesh_changed_info = true
  # system_info = 'framework mesh aux nonlinear execution'
  # []
[]
