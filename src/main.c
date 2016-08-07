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

  const char *const short_options = "hvc:pgzlsuBa:wV";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"pause", 0, NULL, 'p'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"restore", 0, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"bypass", 1, NULL, 'B'},
    {"add-waste", 1, NULL, 'a'},
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
  const char *W_addition = NULL;

  wasteNum = 0;
  curWasteNum = 0;

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
    case 'a':
      W_addition = optarg;
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

  char HOMEDIR[strlen (getenv ("HOME")) + 1];
  strcpy (HOMEDIR, getenv ("HOME"));

  buf_check (HOMEDIR, MP);

  char *data_dir = malloc (MP * sizeof (char));

  buf_check_with_strop (data_dir, HOMEDIR, CPY);

  buf_check_with_strop (data_dir, DATADIR, CAT);


  if (file_exist (data_dir) == 1)
  {
    if (mkdir (data_dir, S_IRWXU) != 0)
    {
      fprintf (stderr, "Error creating configuration directory: %s\n",
               data_dir);
      exit (1);
    }
  }                             // else

  if (W_addition != NULL)
  {
    char configFile[MP];
    if (alt_config != NULL)
    {
      buf_check_with_strop (configFile, alt_config, CPY);
    }
    else
    {
      strcpy (configFile, HOMEDIR);
      strcat (configFile, CFG);
    }
    FILE *fp = fopen (configFile, "a");
    if (fp == NULL)
    {
      fprintf (stderr, "Error appending to configuration file\n");
      exit (1);
    }
    else
    {
      fprintf (fp, "\nWASTE = %s\n", W_addition);
      fclose (fp);
      printf ("%s added to %s\n", W_addition, configFile);
    }
  }

  free (data_dir);

  int i = 0;


  int purge_after = 90;

  /**
   * BEGIN get_config
   */

  struct stat st;
  FILE *cfgPtr;
  wasteNum = 0;
  char *tokenPtr;
  char configFile[MP];
  const unsigned short CFG_MAX_LEN = PATH_MAX + 16;
  char confLine[CFG_MAX_LEN];

  /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
  {
        /**
	 * Besides boundary checking, buf_check_with_strop()
	 * will perform the strcpy or strcat)
	 */

    /* copy $HOMEDIR to configFile */
    buf_check_with_strop (configFile, HOMEDIR, CPY);

    // CFG is the file name of the rmw config file relative to
    // the $HOME directory, defined at the top of rmw.h
    /* create full path to configFile */
    buf_check_with_strop (configFile, CFG, CAT);
  }
  else
    buf_check_with_strop (configFile, alt_config, CPY);

  cfgPtr = fopen (configFile, "r");
  /* if configFile isn't found in home directory,
   *  open the system default */
  if (cfgPtr == NULL)
  {
    /* buf_check_with_strop(configFile, "/etc/rmwrc", CPY); */
    buf_check_with_strop (configFile, SYSCONFDIR "/rmwrc", CPY);
    cfgPtr = fopen (configFile, "r");
    if (cfgPtr == NULL)
    {
      fprintf (stderr, "Configuration file (%s) not found.\nExiting...\n",
               configFile);
      exit (1);
    }
  }

  while (fgets (confLine, CFG_MAX_LEN, cfgPtr) != NULL)
  {
    buf_check (confLine, CFG_MAX_LEN);
    trim (confLine);
    erase_char (' ', confLine);

    if (strncmp (confLine, "purge_after", 11) == 0 || strncmp(confLine, "purgeDays", 9) == 0)
    {

      tokenPtr = strtok (confLine, "=");
      tokenPtr = strtok (NULL, "=");
      // buf_check (tokenPtr, 4096);
      erase_char (' ', tokenPtr);
      buf_check (tokenPtr, 6);
      purge_after = atoi (tokenPtr);
    }
    else
      /* if (strncmp ("PROTECT", confLine, 7) == 0)
         parse_protected (confLine); */

    if (wasteNum < WASTENUM_MAX)
    {
      if (strncmp ("WASTE", confLine, 5) == 0)
      {
        tokenPtr = strtok (confLine, "=");
        tokenPtr = strtok (NULL, "=");

        trim_slash (tokenPtr);
        erase_char (' ', tokenPtr);
        change_HOME (tokenPtr, HOMEDIR);
        buf_check_with_strop (waste[wasteNum].parent, tokenPtr, CAT);

        // Make WASTE/files string
        /* No need to check boundaries for the copy */
        strcpy (waste[wasteNum].files, waste[wasteNum].parent);
        buf_check_with_strop (waste[wasteNum].files, "/files/", CAT);

        // Make WASTE/info string
        /* No need to check boundaries for the copy */
        strcpy (waste[wasteNum].info, waste[wasteNum].parent);
        buf_check_with_strop (waste[wasteNum].info, "/info/", CAT);


        // Create WASTE if it doesn't exit
        waste_check (waste[wasteNum].parent);


        // Create WASTE/files if it doesn't exit
        waste_check (waste[wasteNum].files);

        // Create WASTE/info if it doesn't exit
        waste_check (waste[wasteNum].info);

        // get device number to use later for rename
        lstat (waste[wasteNum].parent, &st);
        W_devnum[wasteNum] = st.st_dev;
        if (list)
          printf ("%s\n", waste[wasteNum].parent);
        wasteNum++;

      }
    }
    else if (wasteNum == WASTENUM_MAX)
      printf ("Maximum number Waste folders reached: %d\n", WASTENUM_MAX);
  }
  fclose (cfgPtr);

  // No WASTE= line found in config file
  if (wasteNum == 0)
  {
    fprintf (stderr, "Configuration file (%s) error; exiting...\n",
             configFile);
    exit (1);
  }

  purge_after;

  // From this point on, wasteNum must not be altered
  /* UPDATE 2016-08-03.. Create a non-global from wasteNum */

/**
 * END get_config
 */

// String appended to duplicate filenames
  char time_str_appended[16];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

// used for DeletionDate in info file
  char time_now[21];
  get_time_string (time_now, 21, "%FT%T");

  int status = 0;

  if (optind < argc && !restoreYes && !select && !undo_last)
  {
    FILE *undo_file_ptr;

    char undo_path[MP];
    bool undo_opened = 0;

    buf_check_with_strop (undo_path, HOMEDIR, CPY);
    buf_check_with_strop (undo_path, UNDOFILE, CAT);


    for (i = optind; i < argc; i++)
    {

      if (pre_rmw_check (argv[i], file_basename, cur_file, waste,
                         bypass) == 0)
      {

        // If the file hasn't been opened yet (should only happen on the
        // first iteration of the loop
        if (!undo_opened)
        {
          undo_file_ptr = fopen (undo_path, "w");
          if (undo_file_ptr == NULL)
          {
            fprintf (stderr, "Unable to open %s for writing\n", undo_path);
            exit (1);
          }
          else
          {
            undo_opened = 1;
          }
        }

        status = remove_to_waste (cur_file, file_basename, waste, time_now,
                                  time_str_appended, undo_file_ptr);
      }
    }                           // end big for


    if (undo_opened)
      fclose (undo_file_ptr);

  }

  else if (restoreYes)
    Restore (argc, argv, optind, time_str_appended);

  else if (select)
    restore_select (waste, time_str_appended);
  else if (undo_last)
    undo_last_rmw (HOMEDIR, time_str_appended);

  else
  {
    if (!purgeYes && !list && W_addition == NULL)
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
      status = purge (purge_after, waste, time_now);
    }

  if (pause)
  {
    printf ("\nPress the any key to continue...\n");
    getchar ();
  }

  return 0;

}
