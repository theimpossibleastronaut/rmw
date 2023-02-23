dep_fts = cc.find_library('fts', required: not cc.has_function('fts_read'))

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_fts]
)
