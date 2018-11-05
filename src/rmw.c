/*
 * rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 * Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
 * Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md
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

#include "rmw.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>

#include "utils_rmw.h"
#include "restore_rmw.h"
#include "trivial_rmw.h"
#include "config_rmw.h"
#include "trashinfo_rmw.h"
#include "purging_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

static ushort
is_time_to_purge (const char *HOMEDIR);

static void
get_time_string (char *tm_str, ushort len, const char *format);

static st_removed*
add_removal (st_removed *removals, const char *file);

static void
create_undo_file (st_removed *removals, st_removed *removals_head,
  const char *HOMEDIR);

static void
dispose_removed (st_removed *node);

int
main (const int argc, char* const argv[])
{
  /* If MP was defined as something other than a number when building */
  if (MP < 1)
  {
    fprintf (stderr, "  :Error: MP must be defined as a number\n");
    exit (1);
  }

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  const char *const short_options = "hvc:goz:lstuwVfir";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"verbose", 0, NULL, 'v'},
    {"translate", 0, NULL, 't'},
    {"config", 1, NULL, 'c'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"orphaned", 0, NULL, 'o'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {"interactive", 0, NULL, 'i'},
    {"recurse", 0, NULL, 'r'},
    {"force", 0, NULL, 'f'},
    {NULL, 0, NULL, 0}
  };

  short int next_option = 0;

  verbose = 0; /* already declared, a global */

  bool cmd_opt_purge = 0;
  bool orphan_chk = 0;
  bool restoreYes = 0;
  bool select = 0;
  bool undo_last = 0;
  extern bool list;

  ushort force = 0;

  extern const char *alt_config;
  alt_config = NULL;

  do
  {
    next_option = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (next_option)
    {
    case 'h':
      print_usage ();
      exit (0);
    case 'v':
      verbose = 1;
      break;
    case 't':
      translate_config ();
      exit (0);
    case 'c':
      alt_config = optarg;
      break;
    case 'l':
      list = 1;
      break;
    case 'g':
      cmd_opt_purge = 1;
      break;
    case 'o':
      orphan_chk = 1;
      break;
    case 'z':
      restoreYes = 1;
      break;
    case 's':
      select = 1;
      break;
    case 'u':
      undo_last = 1;
      break;
    case 'w':
      warranty ();
      break;
    case 'V':
      version ();
      break;
    case 'i':
      printf (_("-i / --interactive: not implemented\n"));
      break;
    case 'r':
      printf (_("-r / --recurse: not implemented\n"));
      break;
    case 'f':
      force++;
      break;
    case '?':
      print_usage ();
      exit (0);
    case -1:
      break;
    default:
      abort ();
    }

  }
  while (next_option != -1);

#ifdef DEBUG
verbose = 1;
#endif

  #ifndef WIN32
    bufchk (getenv ("HOME"), MP);
    const char *HOMEDIR = getenv ("HOME");
  #else
    bufchk (getenv ("LOCALAPPDATA"), MP);
    const char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif

  if (HOMEDIR == NULL)
  {
    /* FIXME: Perhaps there should be an option in the config file so a
     * user can specify a home directory, and pass the $HOME variable
     */
    printf (_("Error: while getting the path to your home directory\n"));
    return 1;
  }

  char *data_dir = calloc(strlen (HOMEDIR) + strlen (DATA_DIR) + 1, 1);
  chk_malloc (data_dir, __func__, __LINE__);
  bufchk (HOMEDIR, MP - strlen (DATA_DIR));
  sprintf (data_dir, "%s%s", HOMEDIR, DATA_DIR);

  ushort created_data_dir;
  created_data_dir = make_dir (data_dir);
  free (data_dir);
  data_dir = NULL;

  /* make_dir now returns MAKE_DIR_SUCCESS or FAILURE
   * We want to assign something more specific that can be used later
   */
  if (created_data_dir)
    created_data_dir = DATA_DIR_CREATED;

  if (!created_data_dir || created_data_dir == DATA_DIR_CREATED)
  {}
  else
  {
    printf (_("\
  :Error: unable to create config and data directory\n\
Please check your configuration file and permissions\n\n"));
    printf (_("Unable to continue. Exiting...\n"));
    return 1;
  }

  /* is reassigned a value in get_config_data() */
  ushort purge_after = 0;

  st_waste *waste_head;
  waste_head = get_config_data (&purge_after, &force, HOMEDIR);

  st_waste *waste_curr = waste_head;

  if (list)
  {
    while (waste_curr != NULL)
    {
      printf ("%s", waste_curr->parent);
      if (waste_curr->removable)
        printf (_(" (removable, attached)"));
      printf ("\n");

      waste_curr = waste_curr->next_node;
    }

    waste_curr = waste_head;
    dispose_waste (waste_curr);
    free (waste_head);
    waste_head = NULL;
    return 0;
  }

  /* helps test the purge function */
  bool use_fake_year = 0;

  if (getenv ("RMWTRASH") != NULL)
  {
    use_fake_year = strcmp (getenv ("RMWTRASH"), "fake-year") ? 0 : 1;
  }

  char *t_fmt;

  if (!use_fake_year)
  {
    t_fmt = "%FT%T";
  }
  else
  {
    printf ("  :test mode: Using fake year\n");
    t_fmt = "1999-%m-%dT%T";
  }

  /* time_now is used for DeletionDate in trashinfo file
    * and for comparison in purge() */
  /* The length of the format above doesn't exceed 21 */
  extern char time_now[];
  get_time_string (time_now, 21, t_fmt);

  /** This if statement spits out a message if someone tries to use -g on
   * the command line but has purge_after set to 0 in the config
   */
  if (cmd_opt_purge && !purge_after)
  /* TRANSLATORS:  "purging" refers to permanently deleting a file or a
   * directory  */
    printf (_("purging is disabled ('purge_after' is set to '0')\n\n"));
  else if (purge_after)
  {
    if (is_time_to_purge (HOMEDIR) == IS_NEW_DAY || cmd_opt_purge)
    {
      if (force)
        purge (purge_after, waste_curr, force, HOMEDIR);
      else if (!created_data_dir)
        printf (_("purge has been skipped: use -f or --force\n"));
    }
  }

  /* String appended to duplicate filenames */
  extern char time_str_appended[];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

  if (orphan_chk)
  {
    waste_curr = waste_head;
    orphan_maint(waste_curr);
    return 0;
  }

  if (select)
  {
    waste_curr = waste_head;
    return restore_select (waste_curr, time_str_appended);
  }

  /* FIXME:
   * undo_last_rmw() should return a value
   */
  if (undo_last)
  {
    waste_curr = waste_head;
    undo_last_rmw (time_str_appended, waste_curr, HOMEDIR);
    return 0;
  }

  int file_arg = 0;

  if (restoreYes)
  {
    /* subtract 1 from optind otherwise the first file in the list isn't
     * restored
     */
    for (file_arg = optind - 1; file_arg < argc; file_arg++)
    {
      waste_curr = waste_head;
      msg_warn_restore(Restore (argv[file_arg], time_str_appended, waste_curr));
    }

    return 0;
  }

  if (optind < argc)
  {
    rmw_target *file = (rmw_target*)malloc (sizeof (rmw_target));
    chk_malloc (file, __func__, __LINE__);

    st_removed *removals = NULL;
    st_removed *removals_head = NULL;

    static ushort main_error;
    int rmwed_files = 0;
    for (file_arg = optind; file_arg < argc; file_arg++)
    {
      bufchk (argv[file_arg], MP);
      strcpy (file->main_argv, argv[file_arg]);

      if (exists (file->main_argv))
      {
        main_error = resolve_path (file->main_argv, file->real_path);
        if (main_error == 1)
          continue;
        else if (main_error > 1)
          break;
      }
      else
      {
        printf (_("File not found: '%s'\n"), file->main_argv);
        continue;
      }

      /**
       * Make some variables
       * get ready for the ReMoval
       */

      static bool match;
      match = 0;

      bufchk (basename (file->main_argv), MP);
      strcpy (file->base_name, basename (file->main_argv));

      /**
       * cycle through wasteDirs to see which one matches
       * device number of file.main_argv. Once found, the ReMoval
       * happens (provided all the tests are passed.
       */
      waste_curr = waste_head;
      while (waste_curr != NULL)
      {
        static struct stat st;
        lstat (file->main_argv, &st);

        if (waste_curr->dev_num == st.st_dev)
        {
          sprintf (file->dest_name, "%s%s", waste_curr->files, file->base_name);

          /* If a duplicate file exists
           */
          if (!exists (file->dest_name))
          {
            // append a time string
            strcat (file->dest_name, time_str_appended);

            // passed to create_trashinfo()
            file->is_duplicate = 1;
          }
          else
            file->is_duplicate = 0;

          bufchk (file->dest_name, MP);
          int rename_res = rename (file->main_argv, file->dest_name);
          if (rename_res == 0)
          {
            if (verbose)
              printf ("'%s' -> '%s'\n", file->main_argv, file->dest_name);

            rmwed_files++;

            int create_ti_res = create_trashinfo (file, waste_curr);
            if (create_ti_res == 0)
            {
              removals = add_removal (removals, file->dest_name);
              if (removals_head == NULL)
                removals_head = removals;
            }
            else
              /* TRANSLATORS: Do not translate ".trashinfo"  */
              printf (_("  :Error: number %d trying to create a .trashinfo file\n"), create_ti_res);
          }
          else
            msg_err_rename (file->main_argv, file->dest_name, __func__, __LINE__);

      /**
       * If we get to this point, it means a WASTE folder was found
       * that matches the file system that file->main_argv was on.
       * Setting match to 1 and breaking from the for loop
       */
          match = 1;
          break;
        }

        waste_curr = waste_curr->next_node;
      }

      if (!match)
      {
        printf (MSG_WARNING);
        printf (_("No suitable filesystem found for \"%s\"\n"), file->main_argv);
      }
    }

    if (removals != NULL)
      create_undo_file (removals, removals_head, HOMEDIR);

    printf (ngettext ("%d file was removed to the waste folder", "%d files were removed to the waste folder",
            rmwed_files), rmwed_files);
    printf ("\n");

    free (file);
    file = NULL;

    if (main_error > 1)
      return main_error;
  }
  else if (!cmd_opt_purge && !created_data_dir)
      printf (_("No filenames or command line options were given\n\
Enter '%s -h' for more information\n"), argv[0]);

  waste_curr = waste_head;
  dispose_waste (waste_curr);
  waste_head = NULL;
  free (waste_head);

  return 0;
}

static void
get_time_string (char *tm_str, ushort len, const char *format)
{
  struct tm *time_ptr;
  time_t now = time (NULL);
  time_ptr = localtime (&now);
  strftime (tm_str, len, format, time_ptr);
  trim (tm_str);
  bufchk (tm_str, len + 1);
}

/*
 * is_time_to_purge()
 *
 * Creates lastpurge file
 * checks to see if purge() was run today
 * if not, returns 1 and writes the day
 * to the lastpurge file.
 */
static ushort
is_time_to_purge (const char *HOMEDIR)
{
  char file_lastpurge[MP];
  sprintf (file_lastpurge, "%s%s", HOMEDIR, PURGE_DAY_FILE);

  bufchk (file_lastpurge, MP);

  char today_dd[3];
  get_time_string (today_dd, 3, "%d");

  FILE *fp;

  if (exists (file_lastpurge))
  {
    fp = fopen (file_lastpurge, "r");
    if (fp == NULL)
    {
      open_err (file_lastpurge, __func__);
      return ERR_OPEN;
    }

    char last_purge_dd[3];

    if (fgets (last_purge_dd, sizeof (last_purge_dd), fp) == NULL)
    {
      printf ("Error: while getting line from %s\n", file_lastpurge);
      perror (__func__);
      close_file (fp, file_lastpurge, __func__);
      return 0;
    }

    bufchk (last_purge_dd, 3);
    trim (last_purge_dd);

    close_file (fp, file_lastpurge, __func__);

    /** if these are the same, purge has already been run today
     */

    if (!strcmp (today_dd, last_purge_dd))
      return IS_SAME_DAY;

    fp = fopen (file_lastpurge, "w");

    if (fp != NULL)
    {
      fprintf (fp, "%s\n", today_dd);
      close_file (fp, file_lastpurge, __func__);
      return IS_NEW_DAY;
    }

    else
    {
      open_err (file_lastpurge, __func__);
      return ERR_OPEN;
    }
  }

  else
  {
    /**
     * Create file if it doesn't exist
     */
    fp = fopen (file_lastpurge, "w");

    if (fp != NULL)
    {
      fprintf (fp, "%s\n", today_dd);

      close_file (fp, file_lastpurge, __func__);

      return IS_NEW_DAY;
    }
    else
    {
      /**
       * if we can't even write this file to the config directory, something
       * is not right. Make it fatal.
       *
       * If the user gets this far though,
       * chances are this error will never be a problem.
       */
      open_err (file_lastpurge, __func__);
      msg_return_code (ERR_OPEN);
      exit (ERR_OPEN);
    }
  }
}

/*
 *
 * name: add_removal()
 *
 * When a file has been successfully rmw'ed, add the filename to
 * a linked list.
 *
 */
static st_removed*
add_removal (st_removed *removals, const char *file)
{
  if (removals == NULL)
  {
    st_removed *temp_node = (st_removed*)malloc (sizeof (st_removed));
    chk_malloc (temp_node, __func__, __LINE__);
    removals = temp_node;
  }
  else
  {
    removals->next_node = (st_removed*)malloc (sizeof (st_removed));
    chk_malloc (removals->next_node, __func__, __LINE__);
    removals = removals->next_node;
  }
  removals->next_node = NULL;
  bufchk (file, MP);
  strcpy (removals->file, file);
  return removals;
}

/*
 *
 * name: create_undo_file()
 *
 * create the undo file by writing out the filenames from a linked list
 * created by add_removal()
 *
 */
static void
create_undo_file (st_removed *removals, st_removed *removals_head, const char *HOMEDIR)
{
  char undo_path[MP];
  bufchk (UNDO_FILE, MP - strlen (HOMEDIR));
  sprintf (undo_path, "%s%s", HOMEDIR, UNDO_FILE);

  FILE *undo_file_ptr = fopen (undo_path, "w");
  if (undo_file_ptr != NULL)
  {
    removals = removals_head;
    while (removals != NULL)
    {
      fprintf (undo_file_ptr, "%s\n", removals->file);
      removals = removals->next_node;
    }
    dispose_removed (removals);
    close_file (undo_file_ptr, undo_path, __func__);
  }
  else
    open_err (undo_path, __func__);
}

/*
 *
 * name: dispose_removed()
 *
 * free the list of removals created by add_removals()
 *
 */
static void
dispose_removed (st_removed *node)
{
  if (node != NULL)
  {
    dispose_removed (node->next_node);
    free (node);
    node = NULL;
  }

  return;
}

/*
 * These are only used by a few functions and don't need to be global.
 * will use "extern" to declare it where it's needed.
 *
 */

bool list;
const char *alt_config = NULL;
char time_now[21];
char time_str_appended[16];

