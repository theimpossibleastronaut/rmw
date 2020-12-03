#include <purging_rmw.h>
#include <assert.h>

#undef RMDIR_MAX_DEPTH
#define RMDIR_MAX_DEPTH 32

int main (void)
{
  char dir_rmdir_test[LEN_MAX_PATH];
  assert (getcwd (dir_rmdir_test, LEN_MAX_PATH) != NULL);

  assert (5 == 33);

  return 0;
}
