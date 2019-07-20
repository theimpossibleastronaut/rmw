
const char *HOMEDIR = "/home/andy";

/*
 * We include the C file here because realize_home() is declared there statically.
 * Because of that, it can't be used from the library.
 *
 */
#include <config_rmw.c>
#include <assert.h>

static void
test_realize_home(void)
{
  char *config_line = malloc (MP);
  chk_malloc (config_line, __func__, __LINE__);

  strcpy (config_line, "$HOME/.trash.rmw/");
  realize_home (config_line);
  printf ("%s\n", config_line);
  assert (!strcmp (config_line, "/home/andy/.trash.rmw"));

  strcpy (config_line, "~/.trash.rmw/");
  printf ("%s\n", config_line);
  realize_home (config_line);
  assert (!strcmp (config_line, "/home/andy/.trash.rmw"));
}


static void
test_del_char_shift_left (void)
{
  char *config_line = malloc (MP);
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

  test_realize_home ();

  test_del_char_shift_left();

  return 0;
}
