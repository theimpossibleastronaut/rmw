/*
 * config_rmw.c
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include "main.h"
#include "config_rmw.h"
#include "utils_rmw.h"
#include "strings_rmw.h"

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
  fputs (_("\
# NOTE: If two WASTE folders are on the same file system, rmw will move files\n\
# to the first WASTE folder listed, ignoring the second one.\n\
#\n"), stream);
  fputs ("WASTE = $HOME/.local/share/Waste\n", stream);

  fputs (_("\n\
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
#\n"), stream);
  fputs ("# WASTE=$HOME/.local/share/Trash\n", stream);

  fputs (_("\n# A folder can use the $UID variable.\n"), stream);
  fputs ("# WASTE=/mnt/fs/Trash-$UID\n", stream);

  fputs (_("\n\
# Removable media: If a folder has ',removable' appended to it, rmw\n\
# will not try to create it; it must be initially created manually. If\n\
# the folder exists when rmw is run, it will be used; if not, it will be\n\
# skipped Once you create \"example_waste\", rmw will automatically create\n\
# example_waste/info and example_waste/files\n"), stream);
  fputs ("# WASTE=/mnt/sda10000/example_waste, removable", stream);

  fputs (_("\n\
# How many days should files be allowed to stay in the waste folders before\n\
# they are permanently deleted\n\
#\n\
# use '0' to disable purging\n\
#\n"), stream);
  fprintf (stream, "purge_after = %d\n", DEFAULT_PURGE_AFTER);

  fputs (_("\n\
# purge is allowed to run without the '-f' option. If you'd rather\n\
# require the use of '-f', you may uncomment the line below.\n\
#\n"), stream);
  fputs ("# force_required\n", stream);
}


/*!
 * If haystack begins with 'needle', returns a pointer to the first occurrence
 * in the string after 'needle'.
 * ex1: char *ptr = del_char_shift ('/', string);
 * '*string' will still point to 'string[0]'
 *
 * ex2: string = del_char_shift ('/', string);
 * '*string' may change
 */
#ifndef TEST_LIB
static
#endif
char *
del_char_shift_left (const char needle, char *haystack)
{
  char *ptr = haystack;
  if (*ptr != needle)
    return ptr;

  while (*ptr == needle)
    ptr++;

  return ptr;
}


/*
 * replace part of a string, adapted from code by Gazl
 * https://www.linuxquestions.org/questions/showthread.php?&p=5794938#post5794938
*/
#ifndef TEST_LIB
static
#endif
char
*strrepl (char *src, const char *str, char *repl)
{
  // The replacement text may make the returned string shorter or
  // longer than src, so just add the length of all three for the
  // mallocation.
  int req_len = multi_strlen (src, str, repl, NULL) + 1;
  char *dest = malloc (req_len);
  chk_malloc (dest, __func__, __LINE__);

  char *s, *d, *p;

  s = strstr (src, str);
  if (s && *str != '\0')
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

  dest = realloc (dest, strlen (dest) + 1);
  chk_malloc (dest, __func__, __LINE__);
  return dest;
}

/*!
 * If "$HOME", "~", or "$UID" is used in the configuration file, convert it
 * to the literal value.
 */
#ifndef TEST_LIB
static
#endif
void
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
      char *dest = strrepl (str, st_var[i].name, (char*)st_var[i].value);
      bufchk (dest, LEN_MAX_PATH);
      strcpy (str, dest);
      free (dest);

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
 */
#ifndef TEST_LIB
static
#endif
st_waste *
parse_line_waste (st_waste * waste_curr, const char * line_ptr,
                 const rmw_options * cli_user_options, bool fake_media_root)
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

  bool is_attached = exists (tmp_waste_parent_folder);
  if (removable && !is_attached)
  {
    if (cli_user_options->list)
      show_folder_line (tmp_waste_parent_folder, removable, is_attached);

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
  waste_curr->parent = malloc (strlen (tmp_waste_parent_folder) + 1);
  chk_malloc (waste_curr->parent, __func__, __LINE__);
  strcpy (waste_curr->parent, tmp_waste_parent_folder);

  /* and the files... */
  int req_len = multi_strlen (waste_curr->parent, "/files/", NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  waste_curr->files = malloc (req_len);
  chk_malloc (waste_curr->files, __func__, __LINE__);
  snprintf (waste_curr->files, req_len, "%s%s", waste_curr->parent, "/files/");

  if (! exists (waste_curr->files))
  {
    if (make_dir (waste_curr->files) != 0)
    {
      exit (errno);
    }
  }

  /*
   * and the info.
   * No buffer checking needed here if creating the "/files/" dir
   * was ok
   *
   */
  req_len = multi_strlen (waste_curr->parent, "/info/", NULL) + 1;
  waste_curr->info = malloc (req_len);
  chk_malloc (waste_curr->info, __func__, __LINE__);
  snprintf (waste_curr->info, req_len, "%s%s", waste_curr->parent, "/info/");

  if (! exists (waste_curr->info))
  {
    if (make_dir (waste_curr->info) != 0)
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
  struct stat st, mp_st;
  if (!lstat (waste_curr->parent, &st))
  {
    waste_curr->dev_num = st.st_dev;

    /* We're reusing 'tmp' here after every call to dirname()
     * An excerpt from the dirname() man page states:
     *
     * char *dirname(char *path);
     *
     * Both  dirname() and basename() may modify the contents of path, so it may be de‐
     * sirable to pass a copy when calling one of these functions.
     */
    char tmp[LEN_MAX_PATH];
    strcpy (tmp, waste_curr->parent);
    char *media_root_ptr = dirname (tmp);
    waste_curr->media_root = malloc (strlen (media_root_ptr) + 1);
    chk_malloc (waste_curr->media_root, __func__, __LINE__);
    strcpy (waste_curr->media_root, media_root_ptr);
    strcpy (tmp, waste_curr->media_root);
    if (!lstat (dirname (tmp), &mp_st))
    {
      if (mp_st.st_dev == waste_curr->dev_num && !fake_media_root)
      {
        free (waste_curr->media_root);
        waste_curr->media_root = NULL;
      }
    }
    else
      msg_err_lstat(waste_curr->parent, __func__, __LINE__);
  }
  else
    msg_err_lstat(waste_curr->parent, __func__, __LINE__);

  return waste_curr;
}


/*!
 * Determine the exact name of the config file to use, and return a file descriptor.
 */
static FILE *
realize_config_file (st_config * st_config_data, const rmw_options * cli_user_options)
{
  /* If no alternate configuration was specifed (-c) */
  if (cli_user_options->alt_config == NULL)
  {
    const char rel_default_config[] = "rmwrc";

    int req_len = multi_strlen (st_config_data->dir, "/", rel_default_config, NULL);
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    sprintf (st_config_data->file, "%s/%s", st_config_data->dir, rel_default_config);
  }
  else
  {
    bufchk (cli_user_options->alt_config, LEN_MAX_PATH);
    strcpy (st_config_data->file, cli_user_options->alt_config);
  }

#define MSG_USING_CONFIG if (verbose) printf (_("config file: %s\n"), st_config_data->file)

  FILE *fd;

  fd = fopen (st_config_data->file, "r");
  if (fd)
  {
    MSG_USING_CONFIG;
    return fd;
  }

  fd = fopen (st_config_data->file, "w");
  if (fd)
  {
    printf (_("Creating default configuration file:"));
    printf ("\n  %s\n\n", st_config_data->file);

    print_config (fd);
    close_file (fd, st_config_data->file, __func__);

    fd = fopen (st_config_data->file, "r");
    if (fd)
    {
      MSG_USING_CONFIG;
      return fd;
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
  open_err (st_config_data->file, __func__);
  printf (_("Unable to read or write a configuration file.\n"));
  exit (errno);
}


const char*
get_config_home_dir (void)
{
  const char rel_default[] = "/.config";

  const char *xdg_config_home = getenv ("XDG_CONFIG_HOME");

  static const char *ptr;

  if (getenv (STR_ENABLE_TEST) != NULL ||
      (xdg_config_home == NULL && getenv (STR_ENABLE_TEST) == NULL))
  {
    int req_len = multi_strlen (HOMEDIR, rel_default, NULL);
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    static char config_home[LEN_MAX_PATH];
    sprintf (config_home, "%s%s", HOMEDIR, rel_default);
    ptr = &config_home[0];
    return ptr;
  }

  bufchk (xdg_config_home, LEN_MAX_PATH);
  return xdg_config_home;
}


/*!
 * Get configuration data (parse the config file)
 */
void
parse_config_file (const rmw_options * cli_user_options, st_config *st_config_data)
{
  FILE *fd = realize_config_file (st_config_data, cli_user_options);

  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
  char line_from_config[LEN_MAX_CFG_LINE];
  while (fgets (line_from_config, sizeof line_from_config, fd) != NULL)
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
     * assign purge_after the value from config file unless set by --purge
     */
    if (strncmp (line_ptr, "purge_after", 11) == 0)
    {
      if (cli_user_options->want_purge <= 0)
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
      else
      {
        st_config_data->purge_after = cli_user_options->want_purge;
      }
    }
    else if (!strcmp (line_ptr, "force_required"))
      st_config_data->force_required = 1;
    else if (strncmp ("WASTE", line_ptr, 5) == 0)
    {
      st_waste *st_new_waste_ptr =
        parse_line_waste (waste_curr, line_ptr, cli_user_options, st_config_data->fake_media_root);
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

  close_file (fd, st_config_data->file, __func__);

  if (waste_curr == NULL)
  {
    print_msg_error ();
    printf (_("no usable WASTE folder could be found\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n"));
    printf ("  " PACKAGE_URL "\n");
    printf ("Unable to continue. Exiting...\n");
    msg_return_code (-1);
    exit (EXIT_FAILURE);
  }

  return;
}


void
init_config_data (st_config *x)
{
  x->dir = get_config_home_dir ();

  if (! exists (x->dir))
    if ((make_dir (x->dir) != 0))
      exit (errno);

  x->st_waste_folder_props_head = NULL;
  /*
   * The default value for purge_after is only used as a last resort,
   * if for some reason purge_after isn't specified in the config file.
   */
  x->purge_after = DEFAULT_PURGE_AFTER;
  x->force_required = 0;

  const char *f = getenv ("RMW_FAKE_MEDIA_ROOT");
  if (f != NULL)
    x->fake_media_root = (strcasecmp (f, "true") == 0) ? true : false;
  else
    x->fake_media_root = false;
  if (verbose)
    printf ("RMW_FAKE_MEDIA_ROOT:%s\n", x->fake_media_root == false ? "false" : "true");
}

void
show_folder_line (const char *folder, const bool is_r, const bool is_attached)
{
  printf ("%s", folder);
  if (is_r && verbose)
  {
    /*
     * These lines are separated to ease translation
     *
     */
    printf (" (");
    printf (_("removable, "));
    /* TRANSLATORS: context - "a mounted device or filesystem is presently attached or mounted" */
    printf ("%s)", is_attached == true ? _("attached") : _("detached"));
  }

  printf ("\n");
}
