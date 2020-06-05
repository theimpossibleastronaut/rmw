#include "globals.h"
#include "strings_rmw.h"
#include "messages_rmw.h"
#include <assert.h>

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

  /* length of "test" will be == LEN_MAX_PATH; bufchk should set errno to 1 */
  char *test = malloc (LEN_MAX_PATH + 1);
  chk_malloc (test, __func__, __LINE__);

  int i;
  for (i = 0; i < LEN_MAX_PATH; i++)
    test[i] = 'a';

  test[i] = '\0';
  bufchk (test, LEN_MAX_PATH);
  assert (errno);
  errno = 0;

   /* length of "test" will be < LEN_MAX_PATH; bufchk should not change errno */
  strcpy (test, "less than BUF_SIZE");
  bufchk (test, LEN_MAX_PATH);
  assert (!errno);


/* length of "test" will be < LEN_MAX_PATH; bufchk should not change errno */
  for (i = 0; i < LEN_MAX_PATH / 2; i++)
    test[i] = 'a';

  test[i] = '\0';
  bufchk (test, LEN_MAX_PATH);
  assert (!errno);

  free (test);
  return 0;
}

