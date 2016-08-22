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

int main (int argc, char *argv[])
{
  struct waste_containers waste[WASTENUM_MAX];

  char file_basename[MP];
  char cur_file[MP];

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

  buf_check  (getenv ("HOME"), MP);
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
  get_config_data(waste, alt_config, HOMEDIR, purge_after, list, waste_ctr,
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

  if (optind < argc && !restoreYes && !select && !undo_last)
  {

    FILE *undo_file_ptr;

    char undo_path[MP];


    buf_check_with_strop (undo_path, HOMEDIR, CPY);
    buf_check_with_strop (undo_path, UNDO_FILE, CAT);

    for (i = optind; i < argc; i++)
    {

      if (!pre_rmw_check (argv[i], file_basename, cur_file, waste,
                         bypass, waste_dirs_total, protected_dir,
                         protected_total))
      {
        /**
         * If the file hasn't been opened yet (should only happen on the
         * first iteration of the loop
         */

        /**
         * Before ReMoving files to Waste, open the undo file for writing
         */
        if (!undo_opened)
        {
          undo_file_ptr = fopen (undo_path, "w");
          undo_opened = 1;


          if (errno)
          {
            perror ("main() - undo file");
            /**
             * if we can't write to the undo file, something is seriously wrong.
             * Free malloc-ed memory and make it a fatal error.
             */
            free (purge_after);
            return 1;
          }
        }

        status = remove_to_waste (cur_file, file_basename, waste, time_now,
                                  time_str_appended, undo_file_ptr,
                                  waste_dirs_total);
      }
    }

    if (undo_opened)
    {
      if (fclose (undo_file_ptr) == EOF)
      {
        perror ("main()");
        fprintf (stderr, "Error %d while closing %s\n", errno, undo_path);
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

  if (pause)
  {
    fprintf (stdout, "\nPress the any key to continue...\n");
    getchar ();
  }

  return 0;

}
