#include "rmw.h"
#include "strings_rmw.h"
#include <assert.h>

/* multi_strlen should return a pre-determined length */

int
main ()
{
  char test_string[] = "this is a test string";
  int start_len = strlen (test_string);
  assert (multi_strlen (4, "this"," is"," a," "test string") == start_len);

  assert (multi_strlen (4, "this"," is"," a," "test string") != 1234);
}
