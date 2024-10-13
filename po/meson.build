dep_intl = []
inc_intl_arg = []
if get_option('nls')
  inc_args = []
  dep_intl = dependency('intl', required: false)
  if not dep_intl.found()
    if host_sys == 'freebsd' or host_sys == 'openbsd' or host_sys == 'dragonfly'
      inc_intl_arg = ['/usr/local/include']
      dep_intl = cc.find_library(
        'intl',
        has_headers: ['libintl.h'],
        dirs: ['/usr/local/lib'],
        header_include_directories: include_directories(inc_intl_arg)
        )
    elif host_sys == 'netbsd'
      inc_intl_arg = ['/usr/pkg/include']
      dep_intl = cc.find_library(
        'intl',
        has_headers: ['libintl.h'],
        dirs: ['/usr/pkg/lib'],
        header_include_directories: include_directories(inc_intl_arg)
        )
    else # darwin, cygwin
      inc_intl_arg = ['/opt/homebrew/opt/gettext/include']
      dep_intl = cc.find_library(
        'intl',
        dirs: ['/opt/homebrew/opt/gettext/lib'],
        has_headers: ['libintl.h'],
        header_include_directories: include_directories(inc_intl_arg)
        )
    endif
  endif
  if dep_intl.found()
    add_project_arguments(inc_args, language : 'c')
    conf.set('ENABLE_NLS', 1)
    conf.set_quoted('LOCALEDIR', localedir)
    i18n = import('i18n')
    i18n.gettext(meson.project_name(),
      args: ['--directory=' + meson.project_source_root()])
  endif
endif
