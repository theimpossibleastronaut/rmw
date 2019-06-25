#include "rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"
#include <assert.h>

#define BUF_SIZE 80

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

  /* length of "test" will be == MP; bufchk should set errno to 1 */
  char *test = malloc (MP + 1);
  chk_malloc (test, __func__, __LINE__);

  int i;
  for (i = 0; i < MP; i++)
    test[i] = 'a';

  test[i] = '\0';
  bufchk (test, MP);
  assert (errno);
  errno = 0;

   /* length of "test" will be < MP; bufchk should not change errno */
  strcpy (test, "less than BUF_SIZE");
  bufchk (test, MP);
  assert (!errno);


/* length of "test" will be < MP; bufchk should not change errno */
  for (i = 0; i < MP / 2; i++)
    test[i] = 'a';

  test[i] = '\0';
  bufchk (test, MP);
  assert (!errno);

  free (test);
  return 0;
}

