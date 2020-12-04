#include "test.h"
#include <purging_rmw.h>

// TODO: Add more tests for the rmdir_recursive function, such as
// removing files that are non-writable. rmw will
// refuse to do that unless -ff is given, at which point, rmw will
// change permission on the file.

int main (void)
{
  char cur_dir[LEN_MAX_PATH];
  assert (getcwd (cur_dir, LEN_MAX_PATH) != NULL);

  // Just an extra trivial check to make sure that the number used for
  // the test matches with what's defined in purging_rmw.h
  assert(RMDIR_MAX_DEPTH == 32);

  char dir_rmdir_test[LEN_MAX_PATH];
  strcpy (dir_rmdir_test, "rmdir_test");

  int i;
  for (i = 0; i < RMDIR_MAX_DEPTH; i++)
  {
    assert (mkdir (dir_rmdir_test, S_IRWXU) == 0);
    chdir (dir_rmdir_test);
  }

  // Go back to the original cwd
  assert (chdir (cur_dir) == 0);

  assert (rmdir_recursive (dir_rmdir_test, 1, 1) == 0);
  assert (rmdir (dir_rmdir_test) == 0);


  // Now try the test again, but creating a number of dirs that exceed MAX_DEPTH
  for (i = 0; i < RMDIR_MAX_DEPTH + 1; i++)
  {
    assert (mkdir (dir_rmdir_test, S_IRWXU) == 0);
    chdir (dir_rmdir_test);
  }

  // Go back to the original cwd
  assert (chdir (cur_dir) == 0);

  // should return an error because MAX_DEPTH was reached
  assert (rmdir_recursive (dir_rmdir_test, 1, 1) == RMDIR_MAX_DEPTH);

  // Change the 'level' argument so it will go one level further
  assert (rmdir_recursive (dir_rmdir_test, 0, 1) == 0);
  assert (rmdir_recursive (dir_rmdir_test, 1, 1) == 0);

  // remove the top directory, which should now be empty
  assert (rmdir (dir_rmdir_test) == 0);

  return 0;
}
