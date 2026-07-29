dep_intl = []
inc_intl_arg = []
if get_option('nls')
  dep_intl = dependency('intl', required: false)

  # Some platforms keep libintl outside the compiler's default search path and
  # ship no pkg-config data for it, so try the usual prefixes by hand. Each
  # entry is a prefix holding include/ and lib/.
  if not dep_intl.found()
    if host_sys == 'freebsd' or host_sys == 'openbsd' or host_sys == 'dragonfly'
      intl_prefixes = ['/usr/local']
    elif host_sys == 'netbsd'
      intl_prefixes = ['/usr/pkg']
    elif host_sys == 'darwin'
      # Homebrew installs to /opt/homebrew on Apple Silicon and /usr/local on
      # Intel; MacPorts uses /opt/local.
      intl_prefixes = [
        '/opt/homebrew/opt/gettext',
        '/usr/local/opt/gettext',
        '/opt/local',
      ]
    else
      intl_prefixes = []
    endif

    fs = import('fs')
    foreach prefix : intl_prefixes
      # include_directories() errors on a path that does not exist, so only
      # consider a prefix that is actually present.
      if not dep_intl.found() and fs.is_dir(prefix / 'include')
        inc_intl_arg = [prefix / 'include']
        dep_intl = cc.find_library(
          'intl',
          required: false,
          has_headers: ['libintl.h'],
          dirs: [prefix / 'lib'],
          header_include_directories: include_directories(inc_intl_arg),
        )
      endif
    endforeach

    if not dep_intl.found()
      inc_intl_arg = []
    endif
  endif

  if dep_intl.found()
    conf.set('ENABLE_NLS', 1)
    conf.set_quoted('LOCALEDIR', localedir)
    i18n = import('i18n')
    i18n.gettext(
      meson.project_name(),
      args: [
        '--add-comments',
        '--keyword=_',
        '--keyword=N_',
        '--directory=' + meson.project_source_root(),
      ],
    )
  else
    warning('libintl not found: building without translations')
  endif
endif
