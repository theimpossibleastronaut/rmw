dep_musl_fts = []
if cc.has_header_symbol('fts.h', 'fts_open')
  dep_musl_fts = cc.find_library('fts', required: false)
else
  error('You may need to install musl-fts-dev.')
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_musl_fts]
)
