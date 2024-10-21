dep_fts = dependency('', required: false)
if not cc.has_function(
  'fts_open',
  prefix: '#include <sys/types.h>\n#include <sys/stat.h>\n#include <fts.h>',
)
  dep_fts = cc.find_library('fts')
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_fts],
)

if get_option('build_tests')
  incdir = include_directories('../../test')

  exe = executable(
    'rm',
    'rm.c',
    link_with: [lib_bsdutilsrm],
    include_directories: incdir,
    c_args: ['-DTEST_LIB'],
  )

  test('rm', exe)
endif
