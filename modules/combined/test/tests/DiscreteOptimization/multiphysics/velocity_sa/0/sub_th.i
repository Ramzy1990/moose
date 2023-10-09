density_f = 2.146e-3 # kg cm-3
# density_m = 1.860e-3 # kg cm-3
cp_f = 1967 # J kg-1 K-1, 6.15 / 2.0e-3
# cp_m = 1760 # J kg-1 K-1, 6.15 / 2.0e-3
k_f = .0553 # W cm-1 K-1
# k_m = .3120 # W cm-1 K-1
gamma = 1 # W cm-3 K-1, Volumetric heat transfer coefficient
viscosity = .5 # dynamic viscosity
alpha = 1 # SUPG stabilization parameter
t_alpha = 2e-4 # K-1, Thermal expansion coefficient
# sink_temperature = 100
# sink_htc = 1

[GlobalParams]
  #   use_exp_form = false
  temperature = temp
  integrate_p_by_parts = true
[]

# [Mesh]
#   type = GeneratedMesh
#   dim = 2

#   #   nx = 200
#   #   ny = 200
#   ## Use a 40-by-40 mesh instead if running on a desktop/small cluster
#   nx = 10
#   ny = 10

#   xmin = 0
#   xmax = 100
#   ymin = 0
#   ymax = 100
#   elem_type = QUAD4
# []

[Mesh]
  [square]
    type = CartesianMeshGenerator
    dim = 2
    ix = '4 4 4 4 4 4 4 4 4 4'
    iy = '4 4 4 4 4 4 4 4 4 4'
    # ix = '2 2 2 2 2 2 2 2 2 2'
    # iy = '2 2 2 2 2 2 2 2 2 2'
    # ix = '4 4 4 4 4 4 4 4 4 4'
    # iy = '4 4 4 4 4 4 4 4 4 4'
    dx = '20 20 20 20 20 20 20 20 20 20'
    dy = '20 20 20 20 20 20 20 20 20 20'
    # elem_type = QUAD4
    subdomain_id = '
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1

1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
1 1 1 1 1  1 1 1 1 1
    '
  []
  [corner_node]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = 'pinned_node'
    coord = '200 200'
  []
[]

# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 1 0 0
# 0 0 0 0 0 0 0 1 0 0
# 0 0 0 0 0 0 0 1 0 0
# 0 0 0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 1 0 0

[Problem]
  type = FEProblem
[]

[Variables]
  [temp]
    family = LAGRANGE
    order = FIRST
    scaling = 1e-3
  []
  [vel]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [p]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [velocity_x]
    family = LAGRANGE
    order = FIRST
  []
  [velocity_y]
    family = LAGRANGE
    order = FIRST
  []
  [heat]
    family = MONOMIAL
    order = FIRST
  []
  # [sink_var]
  #   family = MONOMIAL
  #   order = CONSTANT
  #   # block = 1
  # []
[]

# From Navier Stokes: https://mooseframework.inl.gov/modules/navier_stokes/index.html
# INSAD option, so AD
#
[Kernels]
  #---------
  # Solid
  #---------
  # [sink]
  #   type = CoupledForce
  #   variable = temp
  #   v = sink_var
  #   block = '1'
  # []

  #---------
  # Fluid
  #---------
  [mass]
    type = INSADMass
    variable = p
    # block = '0'
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
    # block = '0'
  []
  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = vel
    # block = '0'
  []
  [momentum_advection]
    type = INSADMomentumAdvection
    variable = vel
    # block = '0'
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    # block = '0'
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    # block = '0'
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    velocity = vel
    # block = '0'
  []
  [buoyancy]
    type = INSADBoussinesqBodyForce
    variable = vel
    # gravity vector, cm s-2
    gravity = '0 -981 0'
    alpha_name = 't_alpha'
    ref_temp = 'temp_ref'
    # block = '0'
  []
  [gravity]
    type = INSADGravityForce
    variable = vel
    # gravity vector, cm s-2
    gravity = '0 -981 0'
    # block = '0'
  []

  [temp_time]
    type = INSADHeatConductionTimeDerivative
    variable = temp
    # block = '0'
  []
  [temp_source]
    type = INSADEnergySource
    variable = temp
    source_variable = heat
    # block = '0'
  []
  [temp_advection]
    type = INSADEnergyAdvection
    variable = temp
    # block = '0'
  []
  [temp_conduction]
    type = ADHeatConduction
    variable = temp
    thermal_conductivity = 'k'
    # block = '0'
  []
  [temp_supg]
    type = INSADEnergySUPG
    variable = temp
    velocity = vel
    # block = '0'
  []

  [temp_sink]
    type = INSADEnergyAmbientConvection
    variable = temp
    # alpha is the heat transfer coefficient.
    alpha = ${gamma}
    T_ambient = 900
    block = '1'
  []

[]

[AuxKernels]
  [vel_x_kernel]
    type = VectorVariableComponentAux
    variable = velocity_x
    vector_variable = vel
    component = 'x'
  []
  [vel_y_kernel]
    type = VectorVariableComponentAux
    variable = velocity_y
    vector_variable = vel
    component = 'y'
  []
  # [sink_aux]
  #   type = ParsedAux
  #   variable = sink_var
  #   coupled_variables = 'temp'
  #   expression = '-${gamma} * (temp - ${sink_temperature})'
  #   block = 1
  #   # execute_on = FORWARD
  # []
[]

[ICs]
  [vel_ic]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = vel
  []
[]

[BCs]
  [no_slip]
    type = VectorDirichletBC
    variable = vel
    boundary = 'bottom left right'
    values = '0 0 0'
  []
  [lid]
    type = VectorDirichletBC
    variable = vel
    boundary = 'top'
    # '50 0 0' corresponds to vel_x = 50 cm s-1 along the top boundary.
    # Change to 10, 20, 30, 40 for different velocity boundary conditions
    # at U_lid = 0.1, 0.2, 0.3, 0.4 m s-1.
    values = '0 0 0'
  []
  [pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  []
[]

[Materials]
  #---------
  # Fluid
  #---------
  [fuel]
    type = GenericConstantMaterial
    prop_names = 'temp_ref'
    prop_values = '900'
    # block = '0'
  []
  [ad_mat]
    type = ADGenericConstantMaterial
    prop_names = 'k rho cp mu t_alpha'
    prop_values = '${k_f} ${density_f} ${cp_f} ${viscosity} ${t_alpha}'
    # block = '0'
  []
  [ins_temp]
    type = INSADStabilized3Eqn
    alpha = ${alpha}
    velocity = vel
    pressure = p
    temperature = temp
    # block = '0'
  []

  #---------
  # Solid
  #---------
  # [block]
  #   type = GenericConstantMaterial
  #   prop_names = 'temp_ref'
  #   prop_values = '900'
  #   block = '1'
  # []
[]

[Executioner]
  type = Transient
  end_time = 30000000

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  #  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_gasm_overlap -sub_pc_factor_shift_type -pc_gasm_blocks -sub_pc_factor_mat_solver_type'
  #  petsc_options_value = 'gasm     lu           200                1                NONZERO                   16             superlu_dist'
  line_search = none

  ## Use the settings below instead if running on a desktop/small cluster
  petsc_options_iname = '-pc_type -sub_pc_type -ksp_gmres_restart -pc_asm_overlap -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      lu           200                1               NONZERO'

  nl_abs_tol = 1e-08
  nl_rel_tol = 1e-08
  error_on_dtmin = false
  dtmin = 1e-6
  dtmax = 10
  steady_state_detection = true
  steady_state_tolerance = 1e-06

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-6
    cutback_factor = 0.5
    growth_factor = 1.5
    optimal_iterations = 10
    iteration_window = 4
    linear_iteration_ratio = 1000
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [bnorm_th]
    type = Receiver
  []

  [cost_function]
    type = ParsedPostprocessor
    pp_names = 'bnorm_th'
    function = '-bnorm_th'
    execute_on = TIMESTEP_END
  []
[]

[VectorPostprocessors]
[]

[MultiApps]
  [ntsApp]
    type = FullSolveMultiApp
    app_type = MoltresApp
    execute_on = timestep_begin
    # positions = '0 0 0'
    input_files = 'sub_nts.i'
    ignore_solve_not_converge = true
  []
[]

[Transfers]
  [to_sub_temp]
    type = MultiAppProjectionTransfer
    #     direction = to_multiapp
    #     multi_app = ntsApp
    to_multi_app = ntsApp
    source_variable = temp
    variable = temp
  []
  [to_sub_vel_x]
    type = MultiAppProjectionTransfer
    #     direction = to_multiapp
    #     multi_app = ntsApp
    to_multi_app = ntsApp
    source_variable = velocity_x
    variable = velocity_x
  []
  [to_sub_vel_y]
    type = MultiAppProjectionTransfer
    #     direction = to_multiapp
    #     multi_app = ntsApp
    to_multi_app = ntsApp
    source_variable = velocity_y
    variable = velocity_y
  []
  [from_sub]
    type = MultiAppProjectionTransfer
    #     direction = from_multiapp
    #     multi_app = ntsApp
    from_multi_app = ntsApp
    source_variable = heat
    variable = heat
  []

  # [mesh_transfer]
  #   type = MultiAppMeshTransfer
  #   to_multi_app = ntsApp
  #   # from_multi_app = forward1
  #   execute_on = 'TIMESTEP_BEGIN'
  # []

  [pp_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ntsApp
    reduction_type = maximum
    from_postprocessor = bnorm
    to_postprocessor = bnorm_th
  []

[]

[Outputs]
  # perf_graph = true
  # print_linear_residuals = true
  [exodus]
    type = Exodus
  []
[]

[Debug]
  # show_var_residual_norms = true
[]
