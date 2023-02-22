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

fts_flag = ''

if cc.has_header_symbol('fts.h', 'fts_open', required: false)
  if not cc.links(code, name : 'fts link check')
    fts_flag = '-lfts'
    if not cc.links(code, args: fts_flag, name: 'fts link check')
      error('You may need to install musl-fts-dev.')
    endif
  endif
endif

lib_bsdutilsrm = static_library(
  'bsdutilsrm',
  ['rm.c', 'strmode.c'],
  link_args: fts_flag
)
