#include "test.h"
#include "globals.h"
#include "utils_rmw.h"

#define BUF_SIZE 80

int
main ()
{
  char str[BUF_SIZE * 3];
  char escaped_path[BUF_SIZE * 3];
  strcpy (str, "string to encode \n\t\v  \f\r");
  escape_url (str, escaped_path, BUF_SIZE * 3);
  printf ("'%s'\n", escaped_path);
  assert (!strcmp (escaped_path, "string%20to%20encode%20%0A%09%0B%20%20%0C%0D"));

  char unescaped_path[BUF_SIZE];
  unescape_url (escaped_path, unescaped_path, BUF_SIZE);
  assert (!strcmp (unescaped_path, "string to encode \n\t\v  \f\r"));
  return 0;
}

