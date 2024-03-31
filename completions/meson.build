fish_comp = dependency('fish', required: false)

if get_option('install_shell_completions')
  fish_files = files(
    'fish/rmw.fish',
  )

  # default location
  fish_install_dir = join_paths(get_option('datadir'), 'fish/vendor_completions.d')

  if fish_comp.found()
    # when needed we overload the variable
    fish_install_dir = fish_comp.get_variable('completionsdir')
  endif

  install_data(fish_files, install_dir: fish_install_dir)
endif

