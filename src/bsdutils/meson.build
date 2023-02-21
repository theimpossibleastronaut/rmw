
# Required on Alpine Linux (and others?)
# TODO: The "required" kwarg should be "true" but only for systems
# that require it, such as Alpine Linux
#
# This doesn't work because meson.get_external_property() returns 'glibc'
#
#dep_musl_fts = []
#libc_type = meson.get_external_property('libc', 'glibc').to_lower()
#if libc_type == 'musl' or libc_type == 'musl-libc'
#  dep_musl_fts = dependency('musl-fts', required: true)
#endif
#message(libc_type)

dep_musl_fts = dependency('musl-fts', required: false)

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_musl_fts]
)
