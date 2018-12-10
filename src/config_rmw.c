/*!
 * @file config_rmw.c
 * @brief Contains functions used to read and parse the configuration file
 */
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

/*!
 * Prints a copy of the default config file to the specified stream. If
 * a translation is available, the output will be translated.
 * @param[in] stream stdout or a file pointer can be used
 * @return void
 */
static void
print_config (FILE *stream)
{
/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\
# NOTE: If two WASTE folders are on the same file system, rmw will move files\n\
# to the first WASTE folder listed, ignoring the second one.\n\
#\n\
WASTE = $HOME/.trash.rmw\n")) < 0)
  msg_err_fatal_fprintf (__func__);

/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\n\
# If you would like this to be your primary trash folder (which usually means\n\
# that it will be the same as your Desktop Trash folder) be sure it precedes\n\
# any other WASTE folders listed in the config file\n\
#\n\
# If you want it checked for files that need purging, simply uncomment\n\
# The line below. Files you move with rmw will go to the folder above by\n\
# default.\n\
#\n\
#WASTE=$HOME/.local/share/Trash\n")) < 0)
  msg_err_fatal_fprintf (__func__);

/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\n\
# Removable media: If a folder has ',removable' appended to it, rmw\n\
# will not try to create it; it must be initially created manually. If\n\
# the folder exists when rmw is run, it will be used; if not, it will be\n\
# skipped Once you create \"example_waste\", rmw will automatically create\n\
# example_waste/info and example_waste/files\n\
#\n\
#WASTE=/mnt/sda10000/example_waste, removable")) < 0)
  msg_err_fatal_fprintf (__func__);

/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\n\
# How many days should files be allowed to stay in the waste folders before\n\
# they are permanently deleted\n\
#\n\
# use '0' to disable purging\n\
#\n\
purge_after = %d\n"), DEFAULT_PURGE_AFTER) < 0)
  msg_err_fatal_fprintf (__func__);

/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\n\
# purge will not run unless `--force` is used at the command line. Uncomment\n\
# the line below if you would like purge to check daily for files that\n\
# that exceed the days specified in purge_after\n\
#\n\
#force_not_required\n")) < 0)
  msg_err_fatal_fprintf (__func__);
}

/*!
 * Print a translation (if available) of the comments in the config file
 * to stdout
 * @param[in] void
 * @return void
 */
void
translate_config (void)
{
  print_config (stdout);
}

/*!
 * Erases characters from the beginning of a string (i.e. shifts the
 * remaining string to the left.
 *
 * What really happens is that the address of str_addr is moved to the right.
 * Because this function alters the address of str_address (rather than characters
 * within the string itself), the address to str_addr (pointer to a pointer)
 * is passed.
 *
 * @param[in] c the character to erase
 * @param[out] &str reference to the string you want to change
 * @return void
 *
 * Example: @code del_char_shift_left ('=', &src_string); @endcode
 */
static void
del_char_shift_left (const char c, char **str)
{
  while (**str == c)
    (*str)++;

  return;
}

/*!
 * If "$HOME" or "~" is used in the configuration file, convert it
 * to the literal value.
 * @param[in] &str reference to the string containing \b $HOME or \b ~
 * @return void
 *
 * Example:
 * @code realize_home (&config_line); @endcode
 */
static void
realize_home (char **str)
{
  trim_char ('/', *str);

  /*
   *
   * FIXME: This will need some work in order to be implemented
   * on Windows
   *
   */
  del_char_shift_left (' ', str);
  if (**str == '~')
    del_char_shift_left ('~', str);
  else if (strncmp (*str, "$HOME", 5) == 0)
    *str += 5;
  else
    return;

  char tmp_str[MP];
  extern const char *HOMEDIR;
  snprintf (tmp_str, MP, "%s%s", HOMEDIR, *str);
  snprintf (*str, MP, "%s", tmp_str);
  return;
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
    char *comma_pos = strchr (value, ',');
    *comma_pos = '\0';

    del_char_shift_left (',', &comma_val);
    del_char_shift_left (' ', &comma_val);

    if (strcmp ("removable", comma_val) == 0)
      removable = 1;
    else
    {
      print_msg_error ();
      printf (_("invalid option in config\n"));
      goto DO_CONT;
    }
  }

  realize_home (&value);

  if (removable && !exists (value))
  {
    extern const bool list;
    if (list)
    {
      /*
       * These lines are separated to ease translation
       *
       */
      printf ("%s", value);
      if (verbose)
      {
        printf (" (");
        printf (_("removable, "));
        printf (_("detached"));
        printf (")");
      }
      printf ("\n");
    }
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
  bufchk (value, MP);
  snprintf (waste_curr->parent, MP, "%s", value);

  /* and the files... */
  bufchk (waste_curr->parent, MP - strlen ("/files/"));
  snprintf (waste_curr->files, MP, "%s%s", waste_curr->parent, "/files/");

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
    print_msg_warn ();
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

    print_config (fp);

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
    trim_white_space (line_from_config);
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
      purge_after = atoi (value);
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
      print_msg_warn ();
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
    print_msg_error ();
    printf (_("no usable WASTE folder could be found\n\
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
