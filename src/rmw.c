/*
 * main.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 * Copyright (C) 2012-2017  Andy Alt (andy400-dev@yahoo.com)
 * Other authors: https://github.com/andy5995/rmw/blob/master/AUTHORS.md
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

#include <sys/stat.h>
#include <getopt.h>

#ifndef INC_RMW_H
#define INC_RMW_H
  #include "rmw.h"
#endif

#include "utils_rmw.h"
#include "restore_rmw.h"
#include "trivial_rmw.h"
#include "config_rmw.h"
#include "trashinfo_rmw.h"
#include "purging_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

static void
get_time_string (char *tm_str, ushort len, const char *format)
{
  struct tm *time_ptr;
  time_t now = time (NULL);
  time_ptr = localtime (&now);
  strftime (tm_str, len, format, time_ptr);
  bufchk (tm_str, len);
  trim (tm_str);
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
is_time_to_purge (void)
{
  #ifndef WIN32
  char *HOMEDIR = getenv ("HOME");
  #else
  char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif
  char file_lastpurge[MP];
  sprintf (file_lastpurge, "%s%s", HOMEDIR, PURGE_DAY_FILE);

  if (bufchk (file_lastpurge, MP))
    return EXIT_BUF_ERR;

  char today_dd[3];
  get_time_string (today_dd, 3, "%d");

  FILE *fp;

  if (!exists (file_lastpurge))
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
      exit (ERR_OPEN);
    }
  }
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  struct waste_containers waste[WASTENUM_MAX];

  const char *const short_options = "hvc:goz:lstuBwVfir";

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
    {"bypass", 0, NULL, 'B'},
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
  bool bypass = 0;
  bool list = 0;

  ushort force = 0;

  const char *alt_config = NULL;

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
    case 'B':
      bypass = 1;
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

  #ifndef WIN32
  char *HOMEDIR = getenv ("HOME");
  #else
  char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif

  if (HOMEDIR != NULL)
  {
    if (bufchk (HOMEDIR, MP))
      return EXIT_BUF_ERR;
  }
  else
  {
    /* FIXME: Perhaps there should be an option in the config file so a
     * user can specify a home directory, and pass the $HOME variable
     */
    printf (_("Error: while getting the path to your home directory\n"));
    return 1;
  }

  char data_dir[MP];

  sprintf (data_dir, "%s%s", HOMEDIR, DATA_DIR);
  if (bufchk (data_dir, MP))
    return EXIT_BUF_ERR;

  const ushort created_data_dir = make_dir (data_dir);
  if (!created_data_dir || created_data_dir == DATA_DIR_CREATED)
  {}
  else
  {
    printf (_("\
  :Error: unable to create config and data directory\n\
Please check your configuration file and permissions\n\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n\n\
  https://github.com/andy5995/rmw/wiki\n\n\
Unable to continue. Exiting...\n"));
    return 1;
  }

  char protected_dir[PROTECT_MAX][MP];

  /* is reassigned a value in get_config_data() */
  ushort purge_after = 0;

  short conf_err =
    get_config_data (waste, alt_config, &purge_after, protected_dir, &force);

  if (conf_err == NO_WASTE_FOLDER)
    return NO_WASTE_FOLDER;
  else if (conf_err == EXIT_BUF_ERR)
    return EXIT_BUF_ERR;

    /* used for DeletionDate in trashinfo file
     * and for comparison in purge() */
  char time_now[21];

  /* helps test the purge function */
  bool use_fake_year = 0;

  if (getenv ("RMWTRASH") != NULL)
  {
    use_fake_year = strcmp (getenv ("RMWTRASH"), "fake-year") ? 0 : 1;
  }

  if (!use_fake_year)
  {
    get_time_string (time_now, 21, "%FT%T");
  }
  else
  {
    get_time_string (time_now, 21, "1999-%m-%dT%T");
  }

  /** This if statement spits out a message if someone tries to use -g on
   * the command line but has purge_after set to 0 in the config
   */
  if (cmd_opt_purge && !purge_after)
  /* TRANSLATORS:  "purging" refers to permanently deleting a file or a
   * directory  */
    printf (_("purging is disabled ('purge_after' is set to '0')\n\n"));
  else if (purge_after)
  {
    if (is_time_to_purge () == IS_NEW_DAY || cmd_opt_purge)
    {
      if (force)
        purge (purge_after, waste, time_now, force);
      else if (!created_data_dir)
        printf (_("purge has been skipped: use -f or --force\n"));
    }
  }

/* I don't know of any reason why --list would be needed with other features,
 * so exit rmw (if -l was passed, it was performed in get_config_data())
 */
  if (list)
  {
    /* FIXME: if a removable device is specified in the config file but not
     * mounted, it will not show up in this list. It would be better if it
     * showed up in this list, but if not mounted, a '!' next to it, or some
     * other indication it's inactive due to not being mounted. Otherwise,
     * the user has to look at the config file to get reminded of what
     * removable devices are specified.
     */
    short ctr = START_WASTE_COUNTER;
    while (strcmp (waste[++ctr].parent, "NULL") != 0)
      printf ("%s\n", waste[ctr].parent);

    return 0;
  }

  /* String appended to duplicate filenames */
  char time_str_appended[16];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

  if (orphan_chk)
  {
    orphan_maint(waste, time_now, time_str_appended);
    return 0;
  }

  /* FIXME:
   * restore_select() should return a value
   */
  if (select)
  {
    restore_select (waste, time_str_appended);
    return 0;
  }

  /* FIXME:
   * undo_last_rmw() should return a value
   */
  if (undo_last)
  {
    undo_last_rmw (time_str_appended, waste);
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
      if (Restore (argv[file_arg], time_str_appended, waste))
      {
        msg_warn_restore();
      }
    }

    return 0;
  }

  struct rmw_target file;

  if (optind < argc)
  {
    bool undo_opened = 0;
    FILE *undo_file_ptr = NULL;
    char undo_path[MP];

    sprintf (undo_path, "%s%s", HOMEDIR, UNDO_FILE);

    /**
     * No files are open at this point, so just using 'return;'
     */
    if (bufchk (undo_path, MP))
      return EXIT_BUF_ERR;

    static ushort main_error;
    main_error = 0;

    int rmwed_files = 0;

    for (file_arg = optind; file_arg < argc; file_arg++)
    {
      strcpy (file.main_argv, argv[file_arg]);

      /**
       * The undo file may be open at this point, so using 'break'
       * After the for loop is the statement to close file, then
       * return EXIT_BUF_ERR
       */
      if ((main_error = bufchk (file.main_argv, MP)))
        break;

      /**
       * Check to see if the file exists, and if so, see if it's protected
       */

      if (exists (file.main_argv) == 0)
      {
        main_error = resolve_path (file.main_argv, file.real_path);

        if (main_error == 1)
          continue;
        else if (main_error > 1)
          break;

        if (!bypass)
        {
          static bool flagged;
          flagged = 0;

          static short prot_ctr;
          prot_ctr = START_WASTE_COUNTER;

          while (strcmp (protected_dir[++prot_ctr], "NULL") != 0)
          {
            if (strncmp (file.real_path, protected_dir[prot_ctr],
                          strlen (protected_dir[prot_ctr])) == 0)
            {
              flagged = 1;
              break;
            }
          }

          if (flagged)
          {
            /* TRANSLATORS:  "protection" is a feature. It means that
             * this program will pass over files that are in
             * "protected" directories, which can be specified in the
             * configuration file.  */
            printf (_("Skipped: %s is in a protected directory\n"),
                file.real_path);
            continue;
          }
        }
      }
      else
      {
        printf (_("File not found: '%s'\n"), file.main_argv);
        continue;
      }

      /**
       * Make some variables
       * get ready for the ReMoval
       */

      static struct stat st;

      static bool match;
      match = 0;

      static short rename_status;
      rename_status = 0;

      static bool info_status;
      info_status = 0;

      file.is_duplicate = 0;

      static ushort current_waste_num;
      current_waste_num = 0;

      strcpy (file.base_name, basename (file.main_argv));

      /**
       * cycle through wasteDirs to see which one matches
       * device number of file.main_argv. Once found, the ReMoval
       * happens (provided all the tests are passed.
       */
      static short dir_num;
      dir_num = START_WASTE_COUNTER;

      while (strcmp (waste[++dir_num].parent, "NULL") != 0)
      {
        lstat (file.main_argv, &st);

        if (waste[dir_num].dev_num == st.st_dev)
        {
          // used by create_trashinfo
          current_waste_num = dir_num;
          sprintf (file.dest_name, "%s%s", waste[dir_num].files, file.base_name);

          /* If a duplicate file exists
           */
          if (exists (file.dest_name) == 0)
          {
            // append a time string
            strcat (file.dest_name, time_str_appended);

            // passed to create_trashinfo()
            file.is_duplicate = 1;
          }

          if ((main_error = bufchk (file.dest_name, MP)))
              break;

          rename_status = rename (file.main_argv, file.dest_name);

          if (rename_status == 0)
          {
            if (verbose)
              printf ("'%s' -> '%s'\n", file.main_argv, file.dest_name);

            rmwed_files++;
            info_status = create_trashinfo (file, waste,
                              time_now, time_str_appended, current_waste_num);

            if (info_status == 0)
            {
                /**
                 * Open undo_file for writing if it hasn't been yet
                 */
                if (!undo_opened)
                {
                  if ((undo_file_ptr = fopen (undo_path, "w")) != NULL)
                    undo_opened = 1;
                  else
                  {
                    open_err (undo_path, __func__);
                    return 1;
                  }
                }
              fprintf (undo_file_ptr, "%s\n", file.dest_name);
            }
            else
              /* TRANSLATORS: Do not translate ".trashinfo"  */
              printf (_("  :Error: number %d trying to create a .trashinfo file\n"), info_status);
          }
          else
          {
            printf (_("  :Error number %d trying to move %s :\n"),
                rename_status, file.main_argv);
            /* FIXME: better to return rename_status. Any side effects
             * if that were done?
             */
            return 1;
          }

      /**
       * If we get to this point, it means a WASTE folder was found
       * that matches the file system file.main_argv was on.
       * Setting match to 1 and breaking from the for loop
       */
          match = 1;
          break;
        }
      }

      if (!match)
      {
        printf (_("  No suitable filesystem found for \"%s\"\n"),
                 file.main_argv);
        return 1;
      }
    }
    if (undo_opened)
      close_file (undo_file_ptr, undo_path, __func__);
    else
      free (undo_file_ptr);

    printf (rmwed_files == 1 ? _("1 file was removed to the waste folder") :
    _("%d files were removed to the waste folder"), rmwed_files);
    printf ("\n");

    if (main_error > 1)
      return main_error;
  }
  else if (!cmd_opt_purge && !created_data_dir)
      printf (_("No filenames or command line options were given\n\
Enter '%s -h' for more information\n"), argv[0]);

  return 0;
}
