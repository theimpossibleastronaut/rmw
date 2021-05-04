#include "test.h"
#include "globals.h"
#include "utils_rmw.h"
#include "purging_rmw.h"

#define BUF_SIZE 80

void
test_isdotdir (void)
{
  assert (isdotdir (".") == true);
  assert (isdotdir ("..") == true);
  assert (isdotdir (".t") == false);
  assert (isdotdir ("...") == false);
  assert (isdotdir ("t.") == false);
  assert (isdotdir (".. ") == false);

  return;
}


void test_rmw_mkdir (void)
{
  const char *subdirs = "foo/bar/21/42";
  char dir[LEN_MAX_PATH];
  sprintf (dir, "%s/%s", HOMEDIR, subdirs);
  assert (rmw_mkdir (dir, S_IRWXU) == 0);
  printf  ("%s\n", dir);
  assert (exists (dir) == true);

  assert (rmw_mkdir (HOMEDIR, S_IRWXU) != 0);
  errno = 0;

  assert (rmdir_recursive (HOMEDIR, 1, 1) == 0);

  // remove the top directory, which should now be empty
  assert (rmdir (HOMEDIR) == 0);

  return;
}

void test_rmw_dirname (void)
{
  char dir[BUF_SIZE];
  strcpy (dir, "/");
  assert (strcmp (rmw_dirname (dir), "/") == 0);

  strcpy (dir, "./foo");
  assert (strcmp (rmw_dirname (dir), ".") == 0);

  strcpy (dir, "../foo");
  assert (strcmp (rmw_dirname (dir), "..") == 0);

  strcpy (dir, "./foo/");
  assert (strcmp (rmw_dirname (dir), ".") == 0);

  strcpy (dir, "./foo/bar/");
  assert (strcmp (rmw_dirname (dir), "./foo") == 0);

  strcpy (dir, "foo/bar/42");
  assert (strcmp (rmw_dirname (dir), "foo/bar") == 0);

  strcpy (dir, "/foo/bar/42");
  assert (strcmp (rmw_dirname (dir), "/foo/bar") == 0);

  strcpy (dir, "..");
  assert (strcmp (rmw_dirname (dir), ".") == 0);

  strcpy (dir, ".");
  assert (strcmp (rmw_dirname (dir), ".") == 0);

  strcpy (dir, "usr");
  assert (strcmp (rmw_dirname (dir), ".") == 0);

  strcpy (dir, "/usr/");
  assert (strcmp (rmw_dirname (dir), "/") == 0);

  strcpy (dir, "//");
  assert (strcmp (rmw_dirname (dir), "/") == 0);

  strcpy (dir, "");
  assert (rmw_dirname (dir) == NULL);

  return;
}

int
main ()
{
  char tmp[LEN_MAX_PATH];
  snprintf (tmp, LEN_MAX_PATH, "%s/%s", HOME_TEST_DIR, "test_utils_dir");
  HOMEDIR = tmp;

  test_isdotdir ();
  test_rmw_mkdir ();
  test_rmw_dirname ();

  char str[BUF_SIZE * 3];
  char escaped_path[BUF_SIZE * 3];
  strcpy (str, "string to encode \n\t\v  \f\r");
  escape_url (str, escaped_path, BUF_SIZE * 3);
  printf ("'%s'\n", escaped_path);
  assert (!strcmp
	  (escaped_path, "string%20to%20encode%20%0A%09%0B%20%20%0C%0D"));

  char unescaped_path[BUF_SIZE];
  unescape_url (escaped_path, unescaped_path, BUF_SIZE);
  assert (!strcmp (unescaped_path, "string to encode \n\t\v  \f\r"));
  return 0;
}
