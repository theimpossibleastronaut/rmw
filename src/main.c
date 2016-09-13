/*
 * main.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andy400-dev@yahoo.com)
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

#include "main.h"

int
main (int argc, char *argv[])
{
  struct waste_containers waste[WASTENUM_MAX];

  const char *const short_options = "hvc:pgz:lsuBwV";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"pause", 0, NULL, 'p'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"bypass", 0, NULL, 'B'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {NULL, 0, NULL, 0}
  };

  short int next_option = 0;

  bool pause = 0;
  bool purgeYes = 0;
  bool restoreYes = 0;
  bool select = 0;
  bool undo_last = 0;
  verbose = 0;
  bool bypass = 0;
  bool list = 0;

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
    case 'c':
      alt_config = optarg;
      break;
    case 'l':
      list = 1;
      break;
    case 'p':
      pause = 1;
      break;
    case 'g':
      purgeYes = 1;
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

  buf_check (getenv ("HOME"), MP);
  char HOMEDIR[strlen (getenv ("HOME")) + 1];
  strcpy (HOMEDIR, getenv ("HOME"));

  buf_check (HOMEDIR, MP);

  char data_dir[MP];

  strcpy (data_dir, HOMEDIR);

  /* Looks like more variable names needs changing */
  buf_check_with_strop (data_dir, DATA_DIR, CAT);

  /* make_dir (data_dir);
   */

  if (make_dir (data_dir))
  {
    fprintf (stderr, "Unable to create config/data directory\n");
    fprintf (stderr, "Please check your configuration file and permissions\n");
    fprintf (stderr, "If you need further help, or to report a possible bug, ");
    fprintf (stderr, "visit the rmw web site at\n");
    fprintf (stderr, "https://github.com/andy5995/rmw/wiki\n");
    fprintf (stderr, "Unable to continue. Exiting...\n");
    return 1;
  }

  int *protected_ctr = malloc (sizeof (*protected_ctr));

  char protected_dir[PROTECT_MAX][MP];

  int *waste_ctr = malloc (sizeof (*waste_ctr));

  unsigned short *purge_after_ptr = malloc (sizeof (*purge_after_ptr));

  short conf_err =
    get_config_data (waste, alt_config, HOMEDIR, purge_after_ptr, list, waste_ctr,
                     protected_dir, protected_ctr);

  const int waste_dirs_total = *waste_ctr;
  free (waste_ctr);

  const int protected_total = *protected_ctr;
  free (protected_ctr);

  const int purge_after = *purge_after_ptr;
  free (purge_after_ptr);

  if (conf_err == NO_WASTE_FOLDER)
    return NO_WASTE_FOLDER;

  /* String appended to duplicate filenames */
  char time_str_appended[16];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

  /* used for DeletionDate in info file */
  char time_now[21];
  get_time_string (time_now, 21, "%FT%T");

  bool undo_opened = 0;
  FILE *undo_file_ptr;
  char undo_path[MP];

  struct rmw_target file;

  int file_arg = 0;

  if (optind < argc && !restoreYes && !select && !undo_last)
  {
    buf_check_with_strop (undo_path, HOMEDIR, CPY);
    buf_check_with_strop (undo_path, UNDO_FILE, CAT);
    int rmwed_files = 0;

    for (file_arg = optind; file_arg < argc; file_arg++)
    {
      strcpy (file.main_argv, argv[file_arg]);
      buf_check (file.main_argv, MP);

      /**
       * Check to see if the file exists, and if so, see if it's protected
       */

      if (!file_not_found (file.main_argv))
      {
        if (!bypass)
        {
          char real_path[MP];

          if (resolve_path (file.main_argv, real_path))
            continue;

          bool flagged = 0;

          short dir_num;

          for (dir_num = 0; dir_num < protected_total; dir_num++)
          {
            if (!strncmp (real_path, protected_dir[dir_num],
                          strlen (protected_dir[dir_num])))
            {
              flagged = 1;
              break;
            }
          }

          if (flagged)
          {
            fprintf (stderr, "File is in protected directory: %s\n", real_path);
            continue;
          }
        }
      }

      else
      {
        fprintf (stderr, "File not found: '%s'\n", file.main_argv);
        continue;
      }

      /**
       * Make some variables
       * get ready for the ReMoval
       */

      struct stat st;
      unsigned short dir_num = 0;
      bool match = 0;
      short rename_status = 0;
      bool info_status = 0;
      file.is_duplicate = 0;

      unsigned short current_waste_num = 0;

      strcpy (file.base_name, basename (file.main_argv));

      /**
       * cycle through wasteDirs to see which one matches
       * device number of file.main_argv. Once found, the ReMoval
       * happens (provided all the tests are passed.
       */

      for (dir_num = 0; dir_num < waste_dirs_total; dir_num++)
      {
        lstat (file.main_argv, &st);
        if (waste[dir_num].dev_num == st.st_dev)
        {
          // used by mkinfo
          current_waste_num = dir_num;
          buf_check_with_strop (file.dest_name, waste[dir_num].files, CPY);
          buf_check_with_strop (file.dest_name, file.base_name, CAT);

          /* If a duplicate file exists
           */
          if (file_not_found (file.dest_name) == 0)
          {
            // append a time string
            buf_check_with_strop (file.dest_name, time_str_appended, CAT);

            // tell make info there's a duplicate
            file.is_duplicate = 1;
          }

          rename_status = rename (file.main_argv, file.dest_name);

          if (rename_status == 0)
          {
            if (verbose)
              printf ("'%s' -> '%s'\n", file.main_argv, file.dest_name);

            rmwed_files++;
            info_status = mkinfo (file, waste,
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
              fprintf (stderr, "mkinfo() returned error %d\n", info_status);
          }

          else
          {
            fprintf (stderr, "Error %d moving %s :\n", rename_status, file.main_argv);
            perror ("remove_to_waste()");
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
        printf ("No suitable filesystem found for \"%s\"\n", file.main_argv);
        return 1;
      }
    }
    if (rmwed_files == 1)
      printf("%d file was ReMoved to Waste\n", rmwed_files);
    else
      printf("%d files were ReMoved to Waste\n", rmwed_files);
  }

  else if (restoreYes)
    Restore (argc, argv, optind, time_str_appended);

  else if (select)
    restore_select (waste, time_str_appended, waste_dirs_total);

  else if (undo_last)
    undo_last_rmw (HOMEDIR, time_str_appended);

  else
  {
    if (!purgeYes && !list)
    {
      printf ("missing filenames or command line options\n");
      printf ("Try '%s -h' for more information\n", argv[0]);
    }
  }

  if (purgeYes != 0 && purge_after == 0)
    printf ("purging is disabled, 'purge_after' is set to '0'\n");

  if (purge_after != 0 && restoreYes == 0 && select == 0)
  {
    if (is_time_to_purge (HOMEDIR) != 0 || purgeYes != 0)
      purge (purge_after, waste, time_now, waste_dirs_total);
  }

  if (undo_opened)
    close_file (undo_file_ptr, undo_path, __func__);

  if (pause)
  {
    fprintf (stdout, "\nPress the any key to continue...\n");
    getchar ();
  }

  return 0;

}
