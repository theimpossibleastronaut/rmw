#include "test.h"
#include <purging_rmw.h>

int
main (void)
{
  char cur_dir[LEN_MAX_PATH];
  assert (getcwd (cur_dir, LEN_MAX_PATH) != NULL);

  // Just an extra trivial check to make sure that the number used for
  // the test matches with what's defined in purging_rmw.h
  assert (RMDIR_MAX_DEPTH == 32);

  char dir_rmdir_test[LEN_MAX_PATH];
  strcpy (dir_rmdir_test, "rmdir_test");

  FILE *fd1;
  FILE *fd2;

  int i;
  for (i = 0; i < RMDIR_MAX_DEPTH; i++)
    {
      assert (mkdir (dir_rmdir_test, S_IRWXU) == 0);

      if (i == RMDIR_MAX_DEPTH - 1)
	{
	  // Make the last directory non-writable
	  assert (chmod (dir_rmdir_test, 00500) == 0);
	}

      // These files can't get created after a dir has permissions set to
      // 00500, so making it conditional
      if (i != RMDIR_MAX_DEPTH - 1)
	{
	  assert (chdir (dir_rmdir_test) == 0);
	  assert ((fd1 = fopen ("test_file1", "w+")) != NULL);
	  assert (fclose (fd1) == 0);
	  assert ((fd2 = fopen ("test file2", "w+")) != NULL);
	  assert (chmod ("test file2", 00400) == 0);
	  assert (fclose (fd2) == 0);
	}
      printf ("%d\n", i);
    }

  // Go back to the original cwd
  assert (chdir (cur_dir) == 0);

  int force = 1;
  int level = 1;

  // Because some of the created files don't have write permission, this
  // should fail the first time when force isn't set to 2
  assert (rmdir_recursive (dir_rmdir_test, level, force) == EACCES);

  force = 2;
  // Now it should pass
  assert (rmdir_recursive (dir_rmdir_test, level, force) == 0);
  assert (rmdir (dir_rmdir_test) == 0);

  // Now try the test again, but creating a number of dirs that exceed MAX_DEPTH
  for (i = 0; i < RMDIR_MAX_DEPTH + 1; i++)
    {
      assert (mkdir (dir_rmdir_test, S_IRWXU) == 0);
      assert (chdir (dir_rmdir_test) == 0);
    }

  // Go back to the original cwd
  assert (chdir (cur_dir) == 0);

  // should return an error because MAX_DEPTH was reached
  assert (rmdir_recursive (dir_rmdir_test, level, force) == RMDIR_MAX_DEPTH);

  // Change the 'level' argument so it will go one level further down
  level = 0;
  assert (rmdir_recursive (dir_rmdir_test, level, force) == 0);

  level = 1;
  assert (rmdir_recursive (dir_rmdir_test, level, force) == 0);

  // remove the top directory, which should now be empty
  assert (rmdir (dir_rmdir_test) == 0);

  return 0;
}
