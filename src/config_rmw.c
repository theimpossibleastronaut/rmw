/*
 * config_rmw.c
 *
 * This file is part of rmw <https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#ifndef INC_RMW_H
#define INC_RMW_H
#include "rmw.h"
#endif

#include "config_rmw.h"
#include "utils_rmw.h"
#include "strings_rmw.h"
#include "trivial_rmw.h"
#include "messages_rmw.h"

static const int DEFAULT_PURGE_AFTER = 90;

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
static void
del_char_shift_left (const char c, char **str)
{
  while (**str == c)
    ++(*str);

  return;
}

/*
 *
 * name: make_home_real
 *
 * if "$HOME" or "~" is used on configuration file
 * change to the value of "HOMEDIR"
 *
 */
static bool
make_home_real (char **str)
{
  bool ok = 0;
  if (*str[0] == '~')
  {
    del_char_shift_left ('~', str);
    ok = 1;
  }
  else if (strncmp (*str, "$HOME", 5) == 0)
  {
    int chars_to_delete;

    for (chars_to_delete = 0; chars_to_delete < 5; chars_to_delete++)
      del_char_shift_left (*str[0], str);

    ok = 1;
  }
  else
    return 0;

  char temp[MP];
  extern const char *HOMEDIR;
  sprintf (temp, "%s%s", HOMEDIR, *str);
  strcpy (*str, temp);

  return ok;
}

/*
 *
 * name: parse_line_waste
 *
 * This function is called when the "WASTE" option is encountered in the
 * config file.
 *
 */
static st_waste *
parse_line_waste (st_waste * waste_curr, char *line_from_config,
                  bool * do_continue)
{
  bool removable = 0;

  char *value = strchr (line_from_config, '=');
  del_char_shift_left ('=', &value);
  del_char_shift_left (' ', &value);
  char rem_opt[CFG_LINE_LEN_MAX];
  bufchk (value, CFG_LINE_LEN_MAX);
  strcpy (rem_opt, value);

  char *comma_val = strchr (rem_opt, ',');

  if (comma_val != NULL)
  {
    del_char_shift_left (',', &comma_val);
    del_char_shift_left (' ', &comma_val);
    unsigned int ctr = 0;

    while (ctr < strlen (value))
    {
      if (value[ctr] == ',')
        value[ctr] = '\0';

      ctr++;
    }

    if (strcmp ("removable", comma_val) == 0)
      removable = 1;
    else
    {
      printf (_("  :Error: invalid option in config\n"));
      goto DO_CONT;
    }
  }

  trim (value);
  trim_slash (value);
  make_home_real (&value);

  if (removable && !exists (value))
  {
    extern const bool list;
    if (list)
      printf (_("%s (removable, detached)\n"), value);
    goto DO_CONT;
  }

  st_waste *temp_node = (st_waste *) malloc (sizeof (st_waste));
  chk_malloc (temp_node, __func__, __LINE__);

  if (waste_curr != NULL)
  {
    waste_curr->next_node = temp_node;
    temp_node->prev_node = waste_curr;
    waste_curr = waste_curr->next_node;
  }
  else
  {
    waste_curr = temp_node;
    waste_curr->prev_node = NULL;
  }

  waste_curr->removable = removable ? true : false;

  /* make the parent... */
  strcpy (waste_curr->parent, value);

  /* and the files... */
  sprintf (waste_curr->files, "%s%s", waste_curr->parent, "/files/");
  bufchk (waste_curr->files, MP);

  if (!exists (waste_curr->files))
  {
    if (make_dir (waste_curr->files) == MAKE_DIR_FAILURE)
    {
      exit (EXIT_FAILURE);
    }
  }

  /* and the info. */
  sprintf (waste_curr->info, "%s%s", waste_curr->parent, "/info/");

  if (!exists (waste_curr->info))
  {
    if (make_dir (waste_curr->info) == MAKE_DIR_FAILURE)
    {
      exit (EXIT_FAILURE);
    }
  }

  /* get device number to use later for rename
   *
   * coverity complains if the return value of lstat isn't checked.
   * Really, if we get to this point, lstat shouldn't have any problem,
   * checking return values is good practice so we'll do it.
   */
  struct stat st;
  if (lstat (waste_curr->parent, &st) == 0)
    waste_curr->dev_num = st.st_dev;
  else
  {
    printf (MSG_WARNING);
    perror ("lstat()");
  }

  return waste_curr;

DO_CONT:
  *do_continue = 1;
  return waste_curr;
}

/*
 *
 * name: realize_config_file
 *
 * determine the exact name of the config file to use, and return a file pointer
 *
 */
static FILE *
realize_config_file (char *config_file)
{
  if (verbose)
    printf ("sysconfdir = " SYSCONFDIR "\n");

  extern const char *HOMEDIR;
  extern const char *alt_config;
  /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
  {
    /**
     * CFG_FILE is the file name of the rmw config file relative to
     * the $HOME directory, defined at the top of rmw.h
     *
     * Create full path to config_file
     */
    bufchk (CFG_FILE, MP - strlen (HOMEDIR));
    sprintf (config_file, "%s%s", HOMEDIR, CFG_FILE);
  }
  else
  {
    bufchk (alt_config, MP);
    strcpy (config_file, alt_config);
  }

#define MSG_USING_CONFIG if (verbose) printf (_("config file: %s\n"), config_file)

  FILE *fp;
  fp = fopen (config_file, "r");
  if (fp != NULL)
  {
    MSG_USING_CONFIG;
    return fp;
  }

  char tmp_config_file[MP];
  strcpy (tmp_config_file, config_file);

  bufchk ("/rmwrc", MP - strlen (SYSCONFDIR));
  sprintf (config_file, "%s%s", SYSCONFDIR, "/rmwrc");

  fp = fopen (config_file, "r");
  if (fp != NULL)
  {
    MSG_USING_CONFIG;
    return fp;
  }

  strcpy (config_file, tmp_config_file);

/*
 * If any changes are made to rmwrc, the text here will need to be updated
 * to match
 */
  fp = fopen (config_file, "w");
  if (fp != NULL)
  {
    printf (_("Creating default configuration file:"));

    /*
     * To reduce clutter in the translation file (rmw.pot), we'll just
     * separate this from the translatable string above.
     *
     */
    printf ("\n  %s\n\n", config_file);

    fprintf (fp, "\n\
# rmw default configuration file\n\
# https://github.com/andy5995/rmw/wiki/\n\
#\n\
# NOTE: If two WASTE folders are on the same file system, rmw will move files\n\
# to the first WASTE folder listed, ignoring the second one.\n\
#\n\
WASTE = $HOME/.trash.rmw\n\
#\n\
\n\
# If you would like this to be your primary trash folder (which usually means\n\
# that it will be the same as your Desktop Trash folder) be sure it precedes\n\
# any other WASTE folders listed in the config file\n\
#\n\
# If you want it checked for files that need purging, simply uncomment\n\
# The line below. Files you move with rmw will go to the folder above by\n\
# default.\n\
#\n\
#WASTE=$HOME/.local/share/Trash\n\
#\n\
\n\
# Removable media: If a folder has ',removable' appended to it, rmw\n\
# will not try to create it; it must be initially created manually. If\n\
# the folder exists when rmw is run, it will be used; if not, it will be\n\
# skipped. Once you create \"example_waste\", rmw will automatically create\n\
# example_waste/info and example_waste/files\n\
#\n\
#WASTE=/mnt/sda10000/example_waste, removable\n\
#\n\
\n\
# How many days should files be allowed to stay in the waste folders before\n\
# they are permanently deleted\n\
#\n\
# use '0' to disable purging\n\
#\n\
purge_after = %d\n\
#\n\
\n\
# purge will not run unless `--force` is used at the command line. Uncomment\n\
# the line below if you would like purge to check daily for files that\n\
# that exceed the days specified in purge_after\n\
#\n\
#force_not_required\n\
#\n\
\n", DEFAULT_PURGE_AFTER);

    close_file (fp, config_file, __func__);
    fp = fopen (config_file, "r");
    if (fp != NULL)
    {
      MSG_USING_CONFIG;
      return fp;
    }
  }

  /*
   * There are only 3 reasons this may happen:
   *
   * 1. a bug in the program
   * 2. the user has insufficient permissions to write to his HOME directory
   * 3. a bad hard drive
   *
   * Therefore, no extra info will be output in the message below until
   * such a time as it seems necessary.
   *
   */
  open_err (config_file, __func__);
  printf (_("Unable to read or write a configuration file.\n"));
  exit (errno);
}

/*
 *
 * name: get_config_data
 *
 * get configuration data (parse the config file), returns a pointer to
 * a linked list, each node containing info about usable "waste"
 * directories
 *
 */
st_waste *
get_config_data (void)
{
  /*
   * The default value for purge_after is only used as a last resort,
   * if for some reason purge_after isn't specified in the config file.
   */
  extern int purge_after;
  extern ushort force;
  purge_after = DEFAULT_PURGE_AFTER;

  char config_file[MP];
  extern const char *alt_config;
  FILE *config_ptr = realize_config_file (config_file);

  st_waste *waste_head = NULL;
  st_waste *waste_curr = NULL;
  char *line_from_config = calloc (CFG_LINE_LEN_MAX + 1, 1);
  while (fgets (line_from_config, CFG_LINE_LEN_MAX, config_ptr) != NULL)
  {
    bool do_continue = 0;
    bufchk (line_from_config, CFG_LINE_LEN_MAX);
    trim (line_from_config);
    del_char_shift_left (' ', &line_from_config);

    switch (*line_from_config)
    {
    case '#':
      continue;
    case '\0':
      continue;
    }
    /**
     * assign purge_after the value from config file
     */
    if (strncmp (line_from_config, "purge_after", 11) == 0 ||
        strncmp (line_from_config, "purgeDays", 9) == 0)
    {
      char *value = strchr (line_from_config, '=');
      del_char_shift_left ('=', &value);
      del_char_shift_left (' ', &value);

      ushort num_value = atoi (value);

      if (num_value <= USHRT_MAX)
        purge_after = num_value;

      else
        /* TRANSLATORS:  "purge_after" is a varible  */
        printf (_("  :Error: invalid purge_after value in configuration\n"));
    }
    else if (strncmp (line_from_config, "force_not_required", 18) == 0)
      if (!force)
        force = 1;
      else
        continue;
    else if (strncmp ("WASTE", line_from_config, 5) == 0)
    {
      waste_curr =
        parse_line_waste (waste_curr, line_from_config, &do_continue);
      if (do_continue)
        continue;
    }
    else if (!strncmp ("PROTECT", line_from_config, 7))
    {
      static bool pctr = 0;
      if (!pctr)
        printf ("The PROTECT feature has been removed.\n");
      pctr = 1;
    }
    else
    {
      fprintf (stderr, MSG_WARNING);
      fprintf (stderr, _("Unknown or invalid option: '%s'\n"),
               line_from_config);
    }

    /*
     * FIXME: Why is this here?? I know it's needed, but I don't remember
     * why it's located in this block. Needs review to see if there's a
     * better location for it.
     *
     */
    if (waste_curr != NULL && waste_head == NULL)
      waste_head = waste_curr;
  }

  free (line_from_config);
  line_from_config = NULL;

  close_file (config_ptr, config_file, __func__);

  if (waste_curr == NULL)
  {
    printf (_("  :Error: no usable WASTE folder could be found\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n"));
    printf ("  " PACKAGE_URL "\n");
    printf ("Unable to continue. Exiting...\n");
    msg_return_code (NO_WASTE_FOLDER);
    exit (NO_WASTE_FOLDER);
  }
  else
    waste_curr->next_node = NULL;

  return waste_head;
}
