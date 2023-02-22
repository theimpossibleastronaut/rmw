code = '''#include <stdio.h>
#include <fts.h>

int main(void)
{
  char *const fts_args[] = { ".", NULL };
  FTS *tree = fts_open(fts_args, FTS_SEEDOT, 0);
  if (tree)
    return fts_close(tree);
  return -1;
}'''

dep_musl_fts = []

if cc.has_header_symbol('fts.h', 'fts_open', required: false)
  if not cc.links(code, name : 'fts link check')
    dep_musl_fts = dependency('musl-fts', required: true)
  endif
else
  error('You may need to install musl-fts-dev.')
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  dependencies: [dep_musl_fts]
)
