#include "globals.h"
#include "strings_rmw.h"
#include "messages_rmw.h"
#include <assert.h>

#define BUF_SIZE 80

int
main ()
{
  char *test = malloc (BUF_SIZE + 1);
  chk_malloc (test, __func__, __LINE__);
  test = NULL;
  trim_white_space (test);
  assert (errno);
  errno = 0;

  test = realloc (test, BUF_SIZE + 1);
  chk_malloc (test, __func__, __LINE__);
  test[0] = '\0';
  trim_white_space (test);
  assert (errno);
  errno = 0;

  strcpy (test, " \n\t\v\f\r ");
  trim_white_space (test);
  assert (!strcmp (test, ""));

  /* fails if \b is present */
  strcpy (test, "Hello World\n\t\v\f\r ");
  trim_white_space (test);
  printf ("'%s'\n", test);
  assert (!strcmp (test, "Hello World"));

  strcpy (test, "Hello World\n\t\v stop\f\r ");
  trim_white_space (test);
  printf ("'%s'\n", test);
  assert (!strcmp (test, "Hello World\n\t\v stop"));

  free (test);

  return 0;
}
