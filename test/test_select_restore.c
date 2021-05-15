#include "test.h"
#include "globals.h"
#include "utils_rmw.h"
#include "bst.h"
#include "restore_rmw.h"

#define BUF_SIZE 80

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct entries {
  char *name;
  off_t size;
  mode_t mode;
}entries;

void test_human_readable_size (void)
{
  const struct entries test_entries[] = {
    { "foo", 1204, S_IFDIR },
    { "bar", 10000, S_IFLNK },
    { "Hello", 28000000000000, S_IFREG },
    { "World On A String", 4096, S_IFDIR },
  };

  int entries_size = ARRAY_SIZE(test_entries);
  int i;

  for (i = 0; i < entries_size; i++)
  {
    int len = strlen (test_entries[i].name);
    char formatted_hr_size[LEN_MAX_FORMATTED_HR_SIZE];
    char *m_dir_entry = create_formatted_str (test_entries[i].size, test_entries[i].mode, test_entries[i].name, len, formatted_hr_size);
    printf ("%s\n", formatted_hr_size);
    free (m_dir_entry);
    switch (i) {
      case 0:
        assert (strcmp (formatted_hr_size, "[1.1 KiB] (D)") == 0);
        break;
      case 1:
        assert (strcmp (formatted_hr_size, "[9.7 KiB] (L)") == 0);
        break;
      case 2:
        assert (strcmp (formatted_hr_size, "[25.4 TiB]") == 0);
        break;
      case 3:
        assert (strcmp (formatted_hr_size, "[4.0 KiB] (D)") == 0);
        break;
      default:
        break;
      }
  }
}

int
main ()
{
  test_human_readable_size ();
  return 0;
}
