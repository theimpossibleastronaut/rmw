src = [
  'rm.c',
  'compat/src/strmode.c',
  'compat/src/pwcache.c',
  'compat/unkown_brother/strlcpy.c',
  ]

lib_bsdcoreutilsrm = static_library(
  'bsdcoreutilsrm',
  src,
  include_directories : include_directories('compat/include'),
  )

dep_rmw = declare_dependency(
  include_directories : include_directories('compat/include')
  )
