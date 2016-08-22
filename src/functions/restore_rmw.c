/*
 * restore_rmw.c
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

#include "rmw.h"

/**
 * FIXME: This apparently needs re-working too. I'm sure it could be
 * written more efficiently
 */
void
Restore (int argc, char *argv[], int optind, char *time_str_appended)
{

  FILE *fp;
  int i;
  char fn_to_restore[MP];
  bool f_state = 0;

  struct filespec
  {
    char *bfn;
    char rp[MP];
    char ip[MP];
  } r;

  char *tokenPtr;
  // adding 5 for the 'Path=' preceding the path.
  char line[MP + 5];

  for (i = optind; i < argc; i++)
  {

    if (file_not_found (argv[i]))
      printf ("%s not found\n", argv[i]);

    else
    {
      buf_check (argv[i], PATH_MAX);
      realpath (argv[i], r.rp);
      r.bfn = basename (argv[i]);

      truncate_str (r.rp, strlen ("files/") + strlen (r.bfn));
      strcpy (r.ip, r.rp);
      strcat (r.ip, "info/");
      strcat (r.ip, r.bfn);
      strcat (r.ip, DOT_TRASHINFO);

      f_state = file_not_found (r.ip);
      if (f_state != 0)
      {
        printf ("no info file found for %s\n", argv[i]);
        break;
      }
      else
      {
        fp = fopen (r.ip, "r");

        if (fp == NULL)
        {
          fprintf (stderr, "Error opening info file: %s\n", r.ip);
          break;
        }

        else
        {
          // Not using the "[Trash Info]" line, but reading the file
          // sequentially
          if (fgets (line, 14, fp) == NULL)
          {
            fprintf (stderr, "Error reading restore file %s\n", r.ip);
            fclose (fp);
            break;
          }
          else if (strncmp (line, "[Trash Info]", 12) != 0)
          {
            fprintf (stderr,
                     "Info file error; format not correct (Line 1): %s\n",
                     r.ip);
            fclose (fp);
            break;
          }

          // adding 5 for the 'Path=' preceding the path.
          if (fgets (line, MP + 6, fp) != NULL)
          {
            tokenPtr = strtok (line, "=");
            tokenPtr = strtok (NULL, "=");
            // tokenPtr now equals the absolute path from the info file
            // truncating '\n'
            buf_check_with_strop (fn_to_restore, tokenPtr, CPY);
            tokenPtr = NULL;
            trim (fn_to_restore);
            fclose (fp);

          }
          else
          {
            printf ("error on line 2 in %s\n", r.ip);
            fclose (fp);
            break;
          }

          /* Check for duplicate filename */
          if (!file_not_found (fn_to_restore))
          {
            buf_check_with_strop (fn_to_restore, time_str_appended, CAT);
            if (verbose == 1)
              fprintf (stdout,
                       "Duplicate filename at destination - appending time string...\n");
          }
          /* end check                                  */

          if (!rename (argv[i], fn_to_restore))
          {
            printf ("+'%s' -> '%s'\n", argv[i], fn_to_restore);
            if (remove (r.ip) != 0)
              fprintf (stderr, "error removing info file: '%s'\n", r.ip);

            else
              if (verbose)
                printf ("-%s\n", r.ip);
          }

          else
            fprintf (stderr, "Restore failed: %s\n", fn_to_restore);

        }
      }
    }

  }
}


void
restore_select (struct waste_containers *waste, char *time_str_appended,
                const int wdt)
{
  struct stat st;
  struct dirent *entry;
  char path_to_file[MP];
  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char */
  char *destiny[1];
  unsigned count = 0;
  int w = 0;
  char input[10];
  char c;
  unsigned char char_count = 0;
  short choice = 0;

  while (w < wdt)
  {
    DIR *dir = opendir (waste[w].files);
    count = 0;

    if (!choice)
      printf ("\t>-- %s --<\n", waste[w].files);

    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".")  || !strcmp (entry->d_name, ".."))
        continue;

      count++;

      if (count == choice || choice == 0)
      {
        buf_check_with_strop (path_to_file, waste[w].files, CPY);
        /* Not yet sure if 'trim' is needed yet; using it
         *  until I get smarter */
        trim (entry->d_name);
        buf_check_with_strop (path_to_file, entry->d_name, CAT);
        trim (path_to_file);
        lstat (path_to_file, &st);
      }

      if (count == choice)
      {
        destiny[0] = path_to_file;
        printf ("\n");
        /* using 0 for third arg so 'for' loop in Restore() will run
         *  at least once */
        Restore (1, destiny, 0, time_str_appended);
        break;
      }

      if (!choice)
      {
        printf ("%3d. %s", count, entry->d_name);

        if (S_ISDIR (st.st_mode))
          printf (" (D)");

        if (S_ISLNK (st.st_mode))
          printf (" (L)");

        printf ("\n");
      }

    }

    closedir (dir);

    if (choice)
      break;

    do
    {

      printf ("Input number to restore, 'enter' to continue, 'q' to quit) ");
      char_count = 0;
      input[0] = '\0';
      choice = 0;

      while ((c = getche ()) != '\n' && char_count < 9 && c >= '0' && c
             <= '9')
        input[char_count++] = c;

      if (c == 'q' && char_count == 0)
        break;

      if (c != '\n')
        char_count = 0;

      if (c == '\n' && char_count == 0)
        break;

      if (char_count == 0)
        printf ("\n");

      else
      {
        input[char_count] = '\0';
        choice = atoi (input);
      }
    }

    while (choice > count || choice < 1);

    /* If user selects 'q' to abort */
    if (c == 'q')
    {
      printf ("\n");
      return;
    }

    if (choice == 0)
      w++;

  }
}
