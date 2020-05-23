
const char *UID = "1000";

/*
 * We include the C file here because realize_home() is declared there statically.
 * Because of that, it can't be used from the library.
 *
 */
#include <config_rmw.c>
#include <assert.h>

static void
test_realize_waste_line(void)
{
  char *config_line = malloc (LEN_MAX_PATH);
  chk_malloc (config_line, __func__, __LINE__);

  strcpy (config_line, "$HOME/.local/share/Waste/");
  realize_waste_line (config_line);
  printf ("%s\n", config_line);
  assert (!strcmp (config_line, "/home/andy/.local/share/Waste"));

  strcpy (config_line, "~/.local/share/Waste/");
  printf ("%s\n", config_line);
  realize_waste_line (config_line);
  assert (!strcmp (config_line, "/home/andy/.local/share/Waste"));
}


static void
test_del_char_shift_left (void)
{
  char *config_line = malloc (LEN_MAX_PATH);
  chk_malloc (config_line, __func__, __LINE__);

  strcpy (config_line, "    Hello, World");
  char *l_ptr = config_line;
  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left ('H', l_ptr);
  assert (!strcmp (l_ptr, "ello, World"));
}

int main (int argc, char *argv[])
{

  test_realize_waste_line ();

  test_del_char_shift_left();

  return 0;
}
