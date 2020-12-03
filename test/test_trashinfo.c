#include <assert.h>
#include "globals.h"
#include "trashinfo_rmw.h"

#define BUF_SIZE 80

int
main ()
{
  init_trashinfo_spec ();

  assert (strcmp (st_trashinfo_spec[TI_HEADER].str, "[Trash Info]") == 0);
  assert (strcmp (st_trashinfo_spec[TI_PATH_LINE].str, "Path=") == 0);
  assert (strcmp (st_trashinfo_spec[TI_DATE_LINE].str, "DeletionDate=") == 0);

  return 0;
}

