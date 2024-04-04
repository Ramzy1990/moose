[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  parameter_names = 'Rinner Router'
  num_values = '1 1'
  initial_condition = '0; 0'
  objective_name = 'objective'
[]

# TAO Nelder Mead, derivative free, unconstrained minimization
[Executioner]
  type = Optimize
  tao_solver = taonm
  # More information on the options can be found at:
  # https://petsc.org/release/manualpages/Tao/TAONM/
  # https://petsc.org/release/manual/tao/#nelder-mead-simplex-method-nm
  petsc_options_iname = '-tao_gatol -tao_cg_delta_max -tao_max_it -tao_nm_lambda'
  petsc_options_value = '1e-6 1e-4 1000 0.001'
  # petsc_options_value = '1e-6 1e-4 500 0.001 0.8'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    # Run on initial so the forward problem can determine number of parameters
    input_files = pin_nts_pure.i
    execute_on = 'FORWARD'
  []
[]  

[Transfers]
  [to_forward_Rinner]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/Rinner'
    to_reporters = 'Ri/value'
  []

  [to_forward_Router]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/Router'
    to_reporters = 'Ro/value'
  []

  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    # params is the reporter name in the subapp, and num_params is the dof_id_type_vector_names variable used in the subapp.
    from_reporters = 'objective/value'
    to_reporters = ' OptimizationReporter/objective'
  []
[]

[Outputs]
  csv = true
[]