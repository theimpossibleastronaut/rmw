#include <stdio.h>
#include "rmw.h"
#include "messages_rmw.h"
#include "config_rmw.h"
#include <assert.h>

int main (int argc, char *argv[])
{
  extern char *HOMEDIR;
  HOMEDIR = malloc (MP);
  chk_malloc (HOMEDIR, __func__, __LINE__);

  strcpy (HOMEDIR, "/home/andy");

  char *config_line = malloc (MP);
  chk_malloc (config_line, __func__, __LINE__);

  strcpy (config_line, "$HOME/.trash.rmw/");
  realize_home (&config_line);
  printf ("%s\n", config_line);
  assert (!strcmp (config_line, "/home/andy/.trash.rmw"));

  strcpy (config_line, "~/.trash.rmw/");
  printf ("%s\n", config_line);
  realize_home (&config_line);
  assert (!strcmp (config_line, "/home/andy/.trash.rmw"));

  return 0;
}
