lib_canfigger = static_library('canfigger', 'canfigger.c')

dep_canfigger = declare_dependency(
  link_with: lib_canfigger,
  include_directories: '.'
  )

install_data(files('README.md', 'LICENSE'),
  install_dir : get_option('docdir') / 'external/canfigger')
