#include "test.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

#define BUF_SIZE 80

int
main ()
{
  int req_len = LEN_MAX_PATH;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);

  req_len = LEN_MAX_PATH - 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);

  req_len = LEN_MAX_PATH + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  assert (errno);
  errno = 0;

  return 0;
}
