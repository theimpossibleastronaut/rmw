#include "rmw.h"
#include "strings_rmw.h"
#include <assert.h>

int
main ()
{
  int req_len = MP;
  bufchk_len (req_len, MP, __func__, __LINE__);

  req_len = MP - 1;
  bufchk_len (req_len, MP, __func__, __LINE__);

  req_len = MP + 1;
  bufchk_len (req_len, MP, __func__, __LINE__);
  assert (errno);
  errno = 0;
}
