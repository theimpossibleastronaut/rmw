
# Required on Alpine Linux (and others?)
# TODO: The "required" kwarg should be "true" but only for systems
# that require it, such as Alpine Linux
dep_musl_fts = dependency('musl-fts', required: false)

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_musl_fts]
)
