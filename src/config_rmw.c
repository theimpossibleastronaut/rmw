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

#include "parse_cli_options.h"
#include "config_rmw.h"
#include "utils_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

static const int DEFAULT_PURGE_AFTER = 0;


/*!
 * Prints a copy of the default config file to the specified stream.
 *
 * If any changes are made to rmwrc.example, the text here will need to be updated
 * to match
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

fputs (_("\n\
# A folder can use the $UID variable.\n\
# WASTE=/mnt/fs/Trash-$UID\n"), stream);

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


/*
 * replace part of a string, adapted from code by Gazl
 * https://www.linuxquestions.org/questions/showthread.php?&p=5794938#post5794938
*/
static void
strrepl (char *dest, char *src, const char *str, char *repl)
{
  char *s, *d, *p;

  s = strstr (src, str);
  if (s)
  {
    d = dest ;
    for (p = src ; p < s ; p++, d++)
        *d = *p ;
    for (p = repl ; *p != '\0' ; p++, d++)
        *d = *p ;
    for (p = s + strlen(str) ; *p != '\0' ; p++, d++)
        *d = *p ;
    *d = '\0' ;
  }
  else
    strcpy(dest, src);

  return;
}

/*!
 * If "$HOME", "~", or "$UID" is used in the configuration file, convert it
 * to the literal value.
 */
static void
realize_waste_line (char *str)
{
  trim_char ('/', str);
  bufchk (str, LEN_MAX_PATH);

  /*
   *
   * FIXME: This will need some work in order to be implemented
   * on Windows
   *
   */

  uid_t uid = geteuid ();
  struct passwd *pwd = getpwuid(uid); /* don't free, see getpwnam() for details */

  if (pwd == NULL)
  {
    print_msg_error ();
    fputs ("Unable to get $UID\n", stderr);
    exit (EXIT_FAILURE);
  }

  /* What's a good length for this? */
  char UID[40];
  snprintf (UID, sizeof UID, "%u", pwd->pw_uid);

  /*
   * I'm not sure the best way to check for a problem here. Likely any problem
   * would be very rare, but I'm inserting a crude error check here just in case
   *
   * -andy5995 2019-07-31
   */
  if (strlen (UID) == sizeof (UID) - 1)
  {
    print_msg_warn ();
    /*
     * If the length of pwd->pw_uid is over 39, it would have been truncated
     * at the snprintf() statement above.
     */
    printf ("Your UID string most likely got truncated:\n%s\n", UID);
  }

  struct st_vars_to_check st_var[] = {
    { "~", HOMEDIR },
    { "$HOME", HOMEDIR },
    { "$UID", UID },
    { NULL, NULL }
  };

  int i = 0;
  while (st_var[i].name != NULL)
  {
    if (strstr (str, st_var[i].name) != NULL)
    {
      char dest[LEN_MAX_CFG_LINE];
      *dest = '\0';
      strrepl (dest, str, st_var[i].name, (char*)st_var[i].value);
      strcpy (str, dest);

      /* check the string again, in case str contains something like
       * $HOME/Trash-$UID (which would be rare, if ever, but... */
      i--;
    }

    i++;
  }

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
                 const rmw_options * cli_user_options)
{
  bool removable = 0;

  char *raw_line = strchr (line_ptr, '=');
  if (raw_line != NULL)
    raw_line++; /* move past the '=' sign */
  else
  {
    print_msg_warn();
    puts ("configuration: A WASTE line must include an '=' sign.");
    return NULL;
  }

  char rem_opt[LEN_MAX_CFG_LINE];
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

    trim_white_space (comma_val);
    if (strcmp ("removable", comma_val) == 0)
      removable = 1;
    else
    {
      print_msg_warn ();
      printf (_("Invalid WASTE option: '%s'\n"), comma_val);
      return NULL;
    }
  }

  raw_line = del_char_shift_left (' ', raw_line);
  char tmp_waste_parent_folder[LEN_MAX_PATH];
  strcpy (tmp_waste_parent_folder, raw_line);
  realize_waste_line (tmp_waste_parent_folder);

  if (removable && !exists (tmp_waste_parent_folder))
  {
    if (cli_user_options->list)
    {
      /*
       * These lines are separated to ease translation
       *
       */
      fputs (tmp_waste_parent_folder, stdout);
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
    return NULL;
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
  bufchk_len (req_len, sizeof waste_curr->files, __func__, __LINE__);
  sprintf (waste_curr->files, "%s%s", waste_curr->parent, "/files/");

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
}


/*!
 * Determine the exact name of the config file to use, and return a file pointer.
 */
static FILE *
realize_config_file (char *config_file, const rmw_options * cli_user_options)
{
  /* If no alternate configuration was specifed (-c) */
  if (cli_user_options->alt_config == NULL)
  {
    /**
     * CFG_FILE is the file name of the rmw config file relative to
     * the $HOME directory, defined at the top of rmw.h
     *
     * Create full path to config_file
     */
    int req_len = multi_strlen (CFG_FILE, HOMEDIR, NULL) + 1;
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    sprintf (config_file, "%s%s", HOMEDIR, CFG_FILE);
  }
  else
  {
    bufchk (cli_user_options->alt_config, LEN_MAX_PATH);
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
 * Get configuration data (parse the config file)
 */
void
get_config_data (const rmw_options * cli_user_options, st_config *st_config_data)
{
  char config_file[LEN_MAX_PATH];
  FILE *config_ptr = realize_config_file (config_file, cli_user_options);

  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
  char line_from_config[LEN_MAX_CFG_LINE];
  while (fgets (line_from_config, sizeof line_from_config, config_ptr) != NULL)
  {
    char *line_ptr = line_from_config;
    bufchk (line_ptr, LEN_MAX_CFG_LINE);
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
        st_config_data->purge_after = atoi (value);
      }
      else
      {
        print_msg_warn();
        puts ("configuration: 'purge_after' line must include an '=' sign.");
      }
    }
    else if (!strcmp (line_ptr, "force_required"))
      st_config_data->force_required = 1;
    else if (strncmp ("WASTE", line_ptr, 5) == 0)
    {
      st_waste *st_new_waste_ptr =
        parse_line_waste (waste_curr, line_ptr, cli_user_options);
      if (st_new_waste_ptr != NULL)
        waste_curr = st_new_waste_ptr;
      else
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

    if (waste_curr != NULL && st_config_data->st_waste_folder_props_head == NULL)
      st_config_data->st_waste_folder_props_head = waste_curr;
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

  return;
}


void
init_config_data (st_config *st_config_data)
{
  st_config_data->st_waste_folder_props_head = NULL;
  /*
   * The default value for purge_after is only used as a last resort,
   * if for some reason purge_after isn't specified in the config file.
   */
  st_config_data->purge_after = DEFAULT_PURGE_AFTER;
  st_config_data->force_required = 0;
}

