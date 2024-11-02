subdir('bsdutils')
disable_curses = ''
dep_menu = []
dep_curses = []
if not get_option('without-curses')
  dep_curses = dependency('curses')
  dep_menu = dependency('menuw', required: false)
  if not dep_menu.found()
    dep_menu = dependency('menu', required: false)
    if not dep_menu.found()
      dep_menu = cc.find_library('menuw')
      if not dep_menu.found()
        dep_menu = cc.find_library('menu')
        if not dep_menu.found()
          conf.set('DISABLE_CURSES', 1)
        endif
      endif
    endif
  endif
else
  conf.set('DISABLE_CURSES', 1)
endif

src = [
  'globals.c',
  'btrfs.c',
  'restore.c',
  'config_rmw.c',
  'parse_cli_options.c',
  'strings_rmw.c',
  'purging.c',
  'messages.c',
  'time_rmw.c',
  'trashinfo.c',
  'utils.c',
]

# Used for btrfs functions
has_statfs = false
has_btrfs_header = false
if host_sys == 'linux'
  if get_option('want_btrfs_clone')
    has_statfs = cc.has_function(
        'statfs',
        prefix: '#include <sys/statfs.h>',
      )
    has_btrfs_header = cc.has_header('linux/btrfs.h')
    if has_statfs and has_btrfs_header
      conf.set('HAVE_LINUX_BTRFS', 1)
    else
      error('''

  : Requirements not met for btrfs clone support.
  : If missing linux/btrfs.h, you probably need to install the linux-headers package.
  : To build without btrfs clone support and skip this check, add
  : "-Dwant_btrfs_clone=false" to the meson setup options.''')
    endif
  endif
endif

config_h = configure_file(output: 'config.h', configuration: conf)

deps_librmw = [dep_intl, canfigger_dep, dep_menu, dep_curses]

lib_rmw = static_library(
  'rmw',
  src,
  include_directories: include_directories(['.', inc_intl_arg]),
  dependencies: deps_librmw,
)

rmw_dep = declare_dependency(
  link_with: [lib_rmw, lib_bsdutilsrm],
  include_directories: include_directories(['.', inc_intl_arg]),
  dependencies: deps_librmw,
)
