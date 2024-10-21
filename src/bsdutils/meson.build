dep_fts = dependency('', required: false)
if not cc.has_function(
  'fts_open',
  prefix: '#include <sys/types.h>\n#include <sys/stat.h>\n#include <fts.h>',
)
  dep_fts = cc.find_library('fts')
endif

src_lib_bsdutilsrm = ['rm.c']

args_lib_bsdutilsrm = []
if cc.has_function('strmode')
  args_lib_bsdutilsrm = ['-DHAVE_STRMODE']
else
  src_lib_bsdutilsrm += 'strmode.c'
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  src_lib_bsdutilsrm,
  dependencies: [dep_fts],
  c_args: args_lib_bsdutilsrm
)

if get_option('build_tests')
  incdir = include_directories('../../test')

  exe = executable(
    'rm',
    'rm.c',
    link_with: [lib_bsdutilsrm],
    include_directories: incdir,
    c_args: ['-DTEST_LIB', args_lib_bsdutilsrm],
  )

  test('rm', exe)
endif
