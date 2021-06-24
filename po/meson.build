gettext_deps = []
if get_option('nls')
  if host_machine.system() == 'linux' or host_machine.system() == 'gnu'
    gettext_deps = []
  elif host_machine.system() == 'freebsd' or host_machine.system() == 'openbsd' or host_machine.system() == 'dragonfly'
    gettext_deps = [cc.find_library('intl', has_headers: ['libintl.h'], dirs: ['/usr/local/lib'], header_args: c_args)]
  elif host_machine.system() == 'netbsd'
    gettext_deps = [cc.find_library('intl', has_headers: ['libintl.h'], dirs: ['/usr/pkg/lib'], header_args: c_args)]
  else # darwin, cygwin
    gettext_deps = [cc.find_library('intl', has_headers: ['libintl.h'])]
  endif

  conf.set('ENABLE_NLS', 1)
  conf.set_quoted('LOCALEDIR', localedir)

  i18n = import('i18n')
  i18n.gettext(meson.project_name(),
    args: ['--directory=' + meson.source_root(), '--sort-output'])
endif
