/*
 * main.c
 *
 * This file is part of rmw (http://rmw.sf.net)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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

  char *data_dir = malloc (MP * sizeof (char));

  strcpy (data_dir, HOMEDIR);

  /* Looks like more variable names needs changing */
  buf_check_with_strop (data_dir, DATA_DIR, CAT);

  check_for_data_dir (data_dir);

  /* ? */
  free (data_dir);

  int i = 0;

  int *protected_ctr = malloc (sizeof (*protected_ctr));
  char protected_dir[PROTECT_MAX][MP];

  int *waste_ctr = malloc (sizeof (*waste_ctr));
  unsigned short int *purge_after = malloc (sizeof (*purge_after));

  int conf_err =
    get_config_data (waste, alt_config, HOMEDIR, purge_after, list, waste_ctr,
                     protected_dir, protected_ctr);

  if (conf_err)
  {
    free (purge_after);
    return 1;
  }

  const int waste_dirs_total = *waste_ctr;
  free (waste_ctr);

  const int protected_total = *protected_ctr;
  free (protected_ctr);

  /* String appended to duplicate filenames */
  char time_str_appended[16];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

  /* used for DeletionDate in info file */
  char time_now[21];
  get_time_string (time_now, 21, "%FT%T");

  int status = 0;

  bool undo_opened = 0;
  FILE *undo_file_ptr;
  char undo_path[MP];

  char cur_file_argv[MP];

  if (optind < argc && !restoreYes && !select && !undo_last)
  {
    buf_check_with_strop (undo_path, HOMEDIR, CPY);
    buf_check_with_strop (undo_path, UNDO_FILE, CAT);

    for (i = optind; i < argc; i++)
    {
      strcpy (cur_file_argv, argv[i]);

      /**
       * Open undo_file for writing
       */
      if (!undo_opened)
      {
        undo_file_ptr = fopen (undo_path, "w");
        undo_opened = 1;

        if (undo_file_ptr == NULL)
        {
          fprintf (stderr, "Error: opening %s :\n", undo_path);
          perror ("main()");
            /**
             * if we can't write to the undo file, something is seriously wrong.
             * Free malloc-ed memory and make it a fatal error.
             */
          free (purge_after);
          return 1;
        }
      }

      buf_check (cur_file_argv, MP);

      /**
       * Check to see if the file exists, and if so, see if it's protected
       */

      bool protected_file = 0;

      if (!file_not_found (cur_file_argv))
      {
        if (!bypass)
        {
          unsigned short waste_dir;
          char rp[MP];

          if (resolve_path (cur_file_argv, rp))
            continue;

          bool flagged = 0;

          short dir_num;

          for (dir_num = 0; dir_num < protected_total; dir_num++)
          {
            if (!strncmp (rp, protected_dir[dir_num],
                          strlen (protected_dir[dir_num])))
            {
              flagged = 1;
              break;
            }
          }

          if (flagged)
          {
            fprintf (stderr, "File is in protected directory: %s\n", rp);
            continue;
          }
        }
      }

      else
      {
        fprintf (stderr, "File not found: '%s'\n", cur_file_argv);
        continue;
      }

      /**
       * Make some variables
       * get ready for the ReMoval
       */

      struct stat st;
      char finalDest[MP];
      int i = 0;
      bool match = 0;
      short statRename = 0;
      bool info_st = 0;
      bool dfn = 0;

      unsigned short curWasteNum = 0;

      char file_basename[MP];
      strcpy (file_basename, basename (cur_file_argv));

      /**
       * cycle through wasteDirs to see which one matches
       * device number of cur_file_argv. Once found, the ReMoval
       * happens (provided all the tests are passed.
       */

      for (i = 0; i < waste_dirs_total; i++)
      {
        lstat (cur_file_argv, &st);
        if (waste[i].dev_num == st.st_dev)
        {
          // used by mkinfo
          curWasteNum = i;
          buf_check_with_strop (finalDest, waste[i].files, CPY);
          buf_check_with_strop (finalDest, file_basename, CAT);
          // If a duplicate file exists

          if (file_not_found (finalDest) == 0)
          {
            // append a time string
            buf_check_with_strop (finalDest, time_str_appended, CAT);

            // tell make info there's a duplicate
            dfn = 1;
          }

          statRename = rename (cur_file_argv, finalDest);

          if (statRename == 0)
          {
            printf ("'%s' -> '%s'\n", cur_file_argv, finalDest);

            info_st = mkinfo (dfn, file_basename, cur_file_argv, waste,
                              time_now, time_str_appended, curWasteNum);

            if (info_st == 0)
              fprintf (undo_file_ptr, "%s\n", finalDest);
            else
              fprintf (stderr, "mkinfo() returned error %d\n", info_st);
          }

          else
          {
            fprintf (stderr, "Error %d moving %s :\n", statRename, cur_file_argv);
            perror ("remove_to_waste()");
            return 1;
          }

      /**
       * If we get to this point, it means a WASTE folder was found
       * that matches the file system cur_file_argv was on.
       * Setting match to 1 and breaking from the for loop
       */
          match = 1;
          break;
        }
      }

      if (!match)
      {
        printf ("No suitable filesystem found for \"%s\"\n", cur_file_argv);
        return 1;
      }
    }
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
    if (purgeD (HOMEDIR) != 0 || purgeYes != 0)
    {
      status = purge (purge_after, waste, time_now, waste_dirs_total);
      free (purge_after);
    }
  }

  if (undo_opened)
  {
    if (fclose (undo_file_ptr) == EOF)
    {
      fprintf (stderr, "Error %d while closing %s\n", errno, undo_path);
      perror ("main()");
    }
  }

  if (pause)
  {
    fprintf (stdout, "\nPress the any key to continue...\n");
    getchar ();
  }

  return 0;

}
