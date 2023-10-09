rho = 2.146e-3 # kg cm-3
# density_m = 1.860e-3 # kg cm-3
cp = 1967 # J kg-1 K-1, 6.15 / 2.0e-3
# cp_m = 1760 # J kg-1 K-1, 6.15 / 2.0e-3
k_f = .0553 # W cm-1 K-1
# k_m = .3120 # W cm-1 K-1
gamma = 1e+02 # W cm-3 K-1, Volumetric heat transfer coefficient
mu = .5 # dynamic viscosity
t_alpha = 2e-4 # K-1, Thermal expansion coefficient
T_init = 900.0
# The upwind and Rhie-Chow interpolation schemes are used here.
# advected_interp_method = 'upwind'
# velocity_interp_method = 'rc'

[GlobalParams]
  #   use_exp_form = false
  # temperature = temp
  # integrate_p_by_parts = true
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Mesh]
  [square]
    type = CartesianMeshGenerator
    dim = 2
    ix = '1 1 1 1 1 1 1 1 1 1'
    iy = '1 1 1 1 1 1 1 1 1 1'
    # ix = '2 2 2 2 2 2 2 2 2 2'
    # iy = '2 2 2 2 2 2 2 2 2 2'
    # ix = '4 4 4 4 4 4 4 4 4 4'
    # iy = '4 4 4 4 4 4 4 4 4 4'
    dx = '20 20 20 20 20 20 20 20 20 20'
    dy = '20 20 20 20 20 20 20 20 20 20'
    # elem_type = QUAD4
    subdomain_id = '
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
'
  []
  # [corner_node]
  #   type = ExtraNodesetGenerator
  #   input = square
  #   new_boundary = 'pinned_node'
  #   coord = '100 100'
  # []
  # [pin]
  #   type = ExtraNodesetGenerator
  #   input = square
  #   new_boundary = 'pin'
  #   nodes = '0'
  # []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []

  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []

  [pressure]
    type = INSFVPressureVariable
  []

  [temp]
    type = INSFVEnergyVariable
    scaling = 1e-3
    initial_condition = ${T_init}
  []

[]

[AuxVariables]
  [heat]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [velocity_magnitude]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = velocity_magnitude
    x = u
    y = v
  []
[]

# From Navier Stokes: https://mooseframework.inl.gov/modules/navier_stokes/index.html
# INSFV option, so FV
#
[FVKernels]
  #---------
  # All!
  #---------
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  #-------------
  # u component
  #-------------
  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = u
    rho = ${rho}
    momentum_component = 'x'
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    rho = ${rho}
    momentum_component = 'x'
  []

  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []

  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [u_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = u
    T_fluid = temp
    # gravity vector, cm s-2
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = 900
    momentum_component = 'x'
    alpha_name = 't_alpha'
  []

  [u_gravity]
    type = INSFVMomentumGravity
    variable = u
    # gravity vector, cm s-2
    gravity = '0 -9.81 0'
    momentum_component = 'x'
    rho = ${rho}
  []

  #-------------
  # v component
  #-------------
  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = v
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []

  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []

  [v_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = v
    T_fluid = temp
    # gravity vector, cm s-2
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = 900
    momentum_component = 'y'
    alpha_name = 't_alpha'
  []

  [v_gravity]
    type = INSFVMomentumGravity
    variable = v
    # gravity vector, cm s-2
    gravity = '0 -9.81 0'
    momentum_component = 'y'
    rho = ${rho}
  []

  #-------------
  # temperature
  #-------------
  [temp_time]
    type = INSFVEnergyTimeDerivative
    variable = temp
    rho = '${rho}'
    cp = '${cp}'
  []

  # Looks like it will need to be constant monomial (i.e., constant polynomial shape function).
  [temp_source]
    type = FVCoupledForce
    variable = temp
    v = heat
  []

  [temp_advection]
    type = INSFVEnergyAdvection
    variable = temp
  []

  [temp_conduction]
    type = FVDiffusion
    variable = temp
    coeff = 'k'
  []

  [temp_sink]
    type = NSFVEnergyAmbientConvection
    variable = temp
    # alpha is the heat transfer coefficient.
    alpha = ${gamma}
    block = '1'
    T_ambient = 900
  []

[]

[FVBCs]

  #-----
  # Lid
  #-----
  [lid]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'top'
    function = 'lid_function'
  []

  #---------
  # No Slip
  #---------
  [u_wall]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'right bottom left'
    function = 0
  []
  [v_wall]
    type = INSFVNoSlipWallBC
    variable = v
    boundary = 'right bottom left'
    function = 0
  []

  #----------
  # Symmetry
  #----------

  # [u_sym]
  #   type = INSFVSymmetryVelocityBC
  #   boundary = 'bottom left'
  #   variable = u
  #   u = u
  #   v = v
  #   mu = 'mu'
  #   momentum_component = 'x'
  # []
  # [v_sym]
  #   type = INSFVSymmetryVelocityBC
  #   boundary = 'bottom left'
  #   variable = v
  #   u = u
  #   v = v
  #   mu = 'mu'
  #   momentum_component = 'y'
  # []
  # [sym_p]
  #   type = INSFVSymmetryPressureBC
  #   boundary = 'bottom left'
  #   variable = pressure
  # []
  # [pressure_pin]
  #   type = FVDirichletBC
  #   variable = p
  #   boundary = 'pinned_node'
  #   value = 0
  # []
[]

[Materials]
  [fuel]
    type = ADGenericFunctorMaterial
    prop_names = 'temp_ref'
    prop_values = '900'
  []
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'k rho cp mu t_alpha'
    prop_values = '${k_f} ${rho} ${cp} ${mu} ${t_alpha}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'temp'
    rho = ${rho}
  []
[]

[Functions]
  [lid_function]
    type = ParsedFunction
    expression = '50'
  []
[]

[Executioner]
  type = Transient
  end_time = 30

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
    execute_on = LINEAR
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
    input_files = 'fv_sub_nts.i'
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
    source_variable = u
    variable = velocity_x
  []
  [to_sub_vel_y]
    type = MultiAppProjectionTransfer
    #     direction = to_multiapp
    #     multi_app = ntsApp
    to_multi_app = ntsApp
    source_variable = v
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

  [mesh_transfer]
    type = MultiAppMeshTransfer
    to_multi_app = ntsApp
    # from_multi_app = forward1
    execute_on = 'TIMESTEP_BEGIN'
  []

  [pp_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ntsApp
    reduction_type = sum
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
