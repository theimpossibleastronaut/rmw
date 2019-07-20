/*!
 * @file config_rmw.c
 * @brief Contains functions used to read and parse the configuration file
 */
/*
 * config_rmw.c
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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
# Note to OSX and Windows users: sending files to 'Desktop' trash\n\
# doesn't work yet\n\
#\n\
# WASTE=$HOME/.local/share/Trash\n")) < 0)
  msg_err_fatal_fprintf (__func__);

/* TRANSLATORS:  Do not translate the last line in this section  */
if (fprintf (stream, _("\n\
# Removable media: If a folder has ',removable' appended to it, rmw\n\
# will not try to create it; it must be initially created manually. If\n\
# the folder exists when rmw is run, it will be used; if not, it will be\n\
# skipped Once you create \"example_waste\", rmw will automatically create\n\
# example_waste/info and example_waste/files\n\
#\n\
# WASTE=/mnt/sda10000/example_waste, removable")) < 0)
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
# purge is allowed to run without the '-f' option. If you'd rather\n\
# require the use of '-f', you may uncomment the line below.\n\
#\n\
# force_required\n\
#\n")) < 0)
  msg_err_fatal_fprintf (__func__);
}


/*!
 * Print a translation (if available) of the comments in the config file
 * to stdout
 */
void
translate_config (void)
{
  print_config (stdout);
}


/*!
 * If a string begins with 'c', returns a pointer to the first occurrence
 * in the string after 'c'
 */
static char *
del_char_shift_left (const char c, char *src_str)
{
  if (*src_str != c)
    return src_str;

  while (*src_str == c)
    src_str++;

  return src_str;
}


/*!
 * If "$HOME" or "~" is used in the configuration file, convert it
 * to the literal value.
 */
static void
realize_home (char *str)
{
  char *str_ptr = str;
  trim_char ('/', str_ptr);
  bufchk (str_ptr, MP);

  /*
   *
   * FIXME: This will need some work in order to be implemented
   * on Windows
   *
   */
  str_ptr = del_char_shift_left (' ', str_ptr);
  if (*str_ptr == '~')
    str_ptr++;
  else if (strncmp (str_ptr, "$HOME", 5) == 0)
    str_ptr += 5;
  else
  {
    // puts ("DEBUG: returning...");
    return;
  }

  char tmp_str[MP];
  extern const char *HOMEDIR;
  int req_len = multi_strlen (HOMEDIR, str_ptr, NULL) + 1;
  bufchk_len (req_len, MP, __func__, __LINE__);
  snprintf (tmp_str, MP, "%s%s", HOMEDIR, str_ptr);
  strcpy (str, tmp_str);
  // puts ("DEBUG:");
  // puts (str);
  return;
}


/*!
 * This function is called when the "WASTE" option is encountered in the
 * config file. The line is parsed and added to the linked list of WASTE
 * folders.
 *
 * @bug <a href="https://github.com/theimpossibleastronaut/rmw/issues/213">rmw is unable to use the system trash folder on a Mac</a>
 */
static st_waste *
parse_line_waste (st_waste * waste_curr, const char * line_ptr,
                  bool * do_continue, const rmw_options * cli_user_options)
{
  bool removable = 0;

  char *raw_line = strchr (line_ptr, '=');
  if (raw_line != NULL)
    raw_line++; /* move past the '=' sign */
  else
  {
    print_msg_warn();
    puts ("configuration: A WASTE line must include an '=' sign.");
    goto DO_CONT;
  }

  char rem_opt[CFG_LINE_LEN_MAX];
  strcpy (rem_opt, raw_line);

  char *comma_val = strchr (rem_opt, ',');
  if (comma_val != NULL)
  {
    // the first part of raw_line before it gets truncated is
    // the waste folder parent
    char *comma_pos = strchr (raw_line, ',');
    *comma_pos = '\0';

    comma_val++;
    comma_val = del_char_shift_left (' ', comma_val);

    trim_white_space (rem_opt);
    // puts ("DEBUG:");
    // puts (rem_opt);
    if (strcmp ("removable", comma_val) == 0)
      removable = 1;
    else
    {
      print_msg_warn ();
      printf (_("invalid option in config\n"));
      goto DO_CONT;
    }
  }

  char tmp_waste_parent_folder[MP];
  strcpy (tmp_waste_parent_folder, raw_line);
  realize_home (tmp_waste_parent_folder);

  if (removable && !exists (tmp_waste_parent_folder))
  {
    if (cli_user_options->list)
    {
      /*
       * These lines are separated to ease translation
       *
       */
      puts (tmp_waste_parent_folder);
      if (verbose)
      {
        printf (" (");
        printf (_("removable, "));
        /* TRANSLATORS: context - "an unmounted device or filesystem is not presently attached or mounted" */
        printf (_("detached"));
        printf (")");
      }
      printf ("\n");
    }
    goto DO_CONT;
  }

  st_waste *temp_node = malloc (sizeof (st_waste));
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
  waste_curr->next_node = NULL;

  waste_curr->removable = removable ? true : false;

  /* make the parent... */
  strcpy (waste_curr->parent, tmp_waste_parent_folder);

  /* and the files... */
  int req_len = multi_strlen (waste_curr->parent, "/files/", NULL) + 1;
  bufchk_len (req_len, MP, __func__, __LINE__);
  snprintf (waste_curr->files, req_len, "%s%s", waste_curr->parent, "/files/");

  if (!exists (waste_curr->files))
  {
    if (make_dir (waste_curr->files) == MAKE_DIR_FAILURE)
    {
      exit (EXIT_FAILURE);
    }
  }

  /*
   * and the info.
   * No buffer checking needed here if creating the "/files/" dir
   * was ok
   *
   */
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
  if (!lstat (waste_curr->parent, &st))
    waste_curr->dev_num = st.st_dev;
  else
    msg_err_lstat(__func__, __LINE__);

  return waste_curr;

DO_CONT:
  *do_continue = 1;
  return waste_curr;
}


/*!
 * Determine the exact name of the config file to use, and return a file pointer.
 */
static FILE *
realize_config_file (char *config_file, const rmw_options * cli_user_options)
{
  if (verbose)
    printf ("sysconfdir = %s\n", SYSCONFDIR);

  extern const char *HOMEDIR;
  /* If no alternate configuration was specifed (-c) */
  if (cli_user_options->alt_config == NULL)
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
    bufchk (cli_user_options->alt_config, MP);
    strcpy (config_file, cli_user_options->alt_config);
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
  bufchk (config_file, MP);
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


/*!
 * Get configuration data (parse the config file), returns a pointer to
 * a linked list, each node containing info about usable "waste"
 * directories.
 */
st_waste *
get_config_data (const rmw_options * cli_user_options)
{
  /*
   * The default value for purge_after is only used as a last resort,
   * if for some reason purge_after isn't specified in the config file.
   */
  extern int purge_after;
  purge_after = DEFAULT_PURGE_AFTER;
  extern bool force_required;

  char config_file[MP];
  FILE *config_ptr = realize_config_file (config_file, cli_user_options);

  st_waste *waste_head = NULL;
  st_waste *waste_curr = NULL;
  char line_from_config[CFG_LINE_LEN_MAX + 1];
  while (fgets (line_from_config, CFG_LINE_LEN_MAX, config_ptr) != NULL)
  {
    bool do_continue = 0;
    char *line_ptr = line_from_config;
    bufchk (line_ptr, CFG_LINE_LEN_MAX);
    trim_white_space (line_ptr);
    line_ptr = del_char_shift_left (' ', line_ptr);

    switch (*line_ptr)
    {
    case '#':
      continue;
    case '\0':
      continue;
    }
    /**
     * assign purge_after the value from config file
     */
    if (strncmp (line_ptr, "purge_after", 11) == 0 ||
        strncmp (line_ptr, "purgeDays", 9) == 0)
    {
      char *value = strchr (line_ptr, '=');
      if (value != NULL)
      {
        value++;
        value = del_char_shift_left (' ', value);
        purge_after = atoi (value);
      }
      else
      {
        print_msg_warn();
        puts ("configuration: 'purge_after' line must include an '=' sign.");
      }
    }
    else if (!strcmp (line_ptr, "force_required"))
      force_required = 1;
    else if (strncmp ("WASTE", line_ptr, 5) == 0)
    {
      waste_curr =
        parse_line_waste (waste_curr, line_ptr, &do_continue, cli_user_options);
      if (do_continue)
        continue;
    }
    else if (!strncmp ("PROTECT", line_ptr, 7))
    {
      /* pctr just prevents this message from being repeated each time
       * "PROTECT" is encountered" */
      static bool pctr = 0;
      if (!pctr)
        printf ("The PROTECT feature has been removed.\n");
      pctr = 1;
    }
    else if (!strcmp ("force_not_required", line_ptr))
      printf ("The 'force_not_required' option has been replaced with 'force_required'.\n");
    else
    {
      print_msg_warn ();
      fprintf (stderr, _("Unknown or invalid option: '%s'\n"),
               line_ptr);
    }

    if (waste_curr != NULL && waste_head == NULL)
      waste_head = waste_curr;
  }

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

  return waste_head;
}
