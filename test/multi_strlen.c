#include "rmw.h"
#include "strings_rmw.h"
#include <assert.h>

/* multi_strlen should return a pre-determined length */

int
main ()
{
  char test_string[] = "this is a test string";
  int start_len = strlen (test_string);
  assert (multi_strlen ("this"," is"," a", " test string", NULL) == start_len);
  return 0;
}
