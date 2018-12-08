#include <stdio.h>
#include "rmw.h"
#include "strings_rmw.h"
#include <assert.h>

int main ()
{
  char test1[] = " \n\t\v\f\r ";
  trim_white_space (test1);
  assert (!strcmp (test1, ""));
  /* fails if \b is present */
  char test[] = "Hello World\n\t\v\f\r ";
  trim_white_space (test);
  printf ("'%s'\n", test);
  assert (!strcmp (test, "Hello World"));
}
