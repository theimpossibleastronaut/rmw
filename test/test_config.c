#include "test.h"
#include <config_rmw.h>
#include <purging_rmw.h>

const char *UID = "1000";

void
test_waste (void)
{
  rmw_options cli_user_options;
  init_rmw_options (&cli_user_options);
  st_config st_config_data;
  init_config_data (&st_config_data);

  char line[] = "WASTE = $HOME/foo/bar/Waste";

  st_waste *waste_curr = st_config_data.st_waste_folder_props_head;

  struct st_waste *st_new_waste_ptr =
    parse_line_waste (waste_curr, line, &cli_user_options,
                      st_config_data.fake_media_root);

  char expected[LEN_MAX_PATH];
  snprintf (expected, LEN_MAX_PATH, "%s/foo/bar/Waste", HOMEDIR);
  assert (strcmp (st_new_waste_ptr->parent, expected) == 0);

  if (!st_config_data.fake_media_root)
    assert (st_new_waste_ptr->media_root == NULL);
  else
    {
      char tmp[LEN_MAX_PATH];
      snprintf (tmp, sizeof tmp, "%s/foo/bar", HOMEDIR);
      // puts (tmp);
      assert (strcmp (st_new_waste_ptr->media_root, tmp) == 0);
    }

  free (st_new_waste_ptr);

  return;
}

void
test_strrepl (void)
{
  char path[LEN_MAX_PATH];
  char *static_path = "/home/foo/bar";
  strcpy (path, static_path);

  char *new_string = strrepl (path, "/foo/bar", "/foo");
  assert (strcmp (new_string, "/home/foo") == 0);
  free (new_string);

  new_string = strrepl (path, "/foo/bar", "/foo/BAR");
  assert (strcmp (new_string, "/home/foo/BAR") == 0);
  free (new_string);

  new_string = strrepl (path, "/foo/bar", "");
  assert (strcmp (new_string, "/home") == 0);
  free (new_string);

  new_string = strrepl (path, "abc", "/home");
  assert (strcmp (new_string, path) == 0);
  free (new_string);

  new_string = strrepl (path, "", "/home");
  puts (new_string);
  assert (strcmp (new_string, path) == 0);
  free (new_string);

  new_string = strrepl (path, "f", "/home/foo/bar");
  assert (strcmp (new_string, "/home//home/foo/baroo/bar") == 0);
  free (new_string);

  assert (strcmp (static_path, path) == 0);

  return;
}

int
main (void)
{
  char tmp[LEN_MAX_PATH];
  snprintf (tmp, LEN_MAX_PATH, "%s/%s", HOME_TEST_DIR, "test_config_dir");
  HOMEDIR = tmp;
  test_waste ();
  test_strrepl ();
  char *config_line = malloc (LEN_MAX_PATH);
  chk_malloc (config_line, __func__, __LINE__);

  strcpy (config_line, "$HOME/.local/share/Waste/");
  realize_waste_line (config_line);
  printf ("%s\n", config_line);
  char expected[LEN_MAX_PATH];
  snprintf (expected, LEN_MAX_PATH, "%s/.local/share/Waste", HOMEDIR);
  assert (!strcmp (config_line, expected));

  strcpy (config_line, "~/.local/share/Waste/");
  printf ("%s\n", config_line);
  realize_waste_line (config_line);
  assert (!strcmp (config_line, expected));

  // test_del_char_shift_left();
  strcpy (config_line, "    Hello, World");
  char *l_ptr = config_line;
  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left ('H', l_ptr);
  assert (!strcmp (l_ptr, "ello, World"));

  free (config_line);

  assert (rmdir_recursive (HOMEDIR, 1, 1) == 0);

  // remove the top directory, which should now be empty
  assert (rmdir (HOMEDIR) == 0);
  return 0;
}
