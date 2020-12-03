#include <config_rmw.h>
#include <assert.h>

const char *UID = "1000";

int main (void)
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

  // test_del_char_shift_left();
  strcpy (config_line, "    Hello, World");
  char *l_ptr = config_line;
  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left (' ', l_ptr);
  assert (!strcmp (l_ptr, "Hello, World"));

  l_ptr = del_char_shift_left ('H', l_ptr);
  assert (!strcmp (l_ptr, "ello, World"));

  return 0;
}
