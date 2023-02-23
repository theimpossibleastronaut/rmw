dep_fts = dependency('', required: false)
if not cc.has_function('fts_open', prefix: '#include <fts.h>')
    dep_fts = cc.find_library('fts')
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_fts]
)
