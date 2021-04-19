#include "test.h"
#include "strings_rmw.h"

/* multi_strlen should return a pre-determined length */

int
main ()
{
  assert (multi_strlen ("this", " is", " a", " test string", NULL) ==
	  strlen ("this is a test string"));
  return 0;
}
