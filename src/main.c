/*
 * main.c
 *
 * This file is part of rmw (https://rmw.sf.net)
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

#include "trivial_funcs.h"
#include "str_funcs.h"
#include "primary_funcs.h"


int
main (int argc, char *argv[])
{

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

  const char *alt_config = NULL;
  const char *W_addition = NULL;

  do
    {
      next_option
	= getopt_long (argc, argv, short_options, long_options, NULL);

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

  buf_check_with_strop (HOMEDIR, getenv ("HOME"), MP);
  trim (HOMEDIR);

  char *data_dir = malloc (MP * sizeof (char));

  buf_check_with_strop (data_dir, getenv ("HOME"), CPY);
  buf_check_with_strop (data_dir, DATADIR, CAT);

  if (file_exist (data_dir) == 1)
    {
      if (mkdir (data_dir, S_IRWXU) != 0)
	{
	  fprintf (stderr, "Error creating configuration directory: %s\n",
		   data_dir);
	  exit (1);
	}
    }				// else

  if (W_addition != NULL)
    {
      char configFile[MP];
      if (alt_config != NULL)
	{
	  buf_check_with_strop (configFile, alt_config, CPY);
	}
      else
	{
	  strcpy (configFile, getenv ("HOME"));
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

  int status = 0;

  get_config (alt_config);

// String appended to duplicate filenames
  get_time_string (appended_time, 16, "_%H%M%S-%y%m%d");

// used for DeletionDate in info file
  get_time_string (present_time, 21, "%FT%T");

  if (optind < argc && !restoreYes && !select && !undo_last)
    {
      char undo_path[MP];
      bool undo_opened = 0;

      buf_check_with_strop (undo_path, getenv ("HOME"), CPY);
      buf_check_with_strop (undo_path, UNDOFILE, CAT);


      for (i = optind; i < argc; i++)
	{

	  if (pre_rmw_check (argv[i]) == 0)
	    {

	      // If the file hasn't been opened yet (should only happen on the
	      // first iteration of the loop
	      if (!undo_opened)
		{
		  undo_file_ptr = fopen (undo_path, "w");
		  if (undo_file_ptr == NULL)
		    {
		      fprintf (stderr, "Unable to open %s for writing\n",
			       undo_path);
		      exit (1);
		    }
		  else
		    {
		      undo_opened = 1;
		    }
		}

	      // reset status to zero
	      status = 0;
	      status = remove_to_waste ();

	      // we only need this once
	      /* if (count == 0)
	         count++; */

	      /* WARNING: info files not compatible with bash rmw,
	       *  cannot be restored with bash rmw */
	      /* if (status == 0)
	         status = mkinfo(TimeStr);

	         else
	         fprintf(stderr,
	         "An error occurred making info file for '%s'\n",
	         argv[i]); */
	    }

	}			// end big for


      if (undo_opened)
	fclose (undo_file_ptr);

    }

  else if (restoreYes)
    Restore (argc, argv, optind);

  else if (select)
    restore_select ();
  else if (undo_last)
    undo_last_rmw ();

  else
    {
      if (!purgeYes && !list && W_addition == NULL)
	{
	  printf ("missing filenames or command line options\n");
	  printf ("Try '%s -h' for more information\n", argv[0]);
	}
    }

  if (purgeYes != 0 && purgeDays == 0)
    printf ("purging is disabled, 'purgeDays' is set to '0'\n");

  if (purgeDays != 0 && restoreYes == 0 && select == 0) {
    if (purgeD () != 0 || purgeYes != 0)
      status = purge ();
    }

// pause before exit

  if (pause)
    {
      printf ("\nPress the any key to continue...\n");
      getchar ();
    }

  return 0;

}
