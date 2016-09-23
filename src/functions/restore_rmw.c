/*
 * restore_rmw.c
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

#include "rmw.h"
#include "restore_rmw.h"

/**
 * FIXME: This apparently needs re-working too. I'm sure it could be
 * written more efficiently
 */
short
Restore (int argc, char *argv[], int optind, char *time_str_appended,
          struct waste_containers *waste, const int waste_dirs_total)
{
  short func_error = 0;

  struct restore
  {
    char *base_name;
    char relative_path[MP];
    char dest[MP];
    char info[MP];
  } file;

  /* adding 5 for the 'Path=' preceding the path.
   * multiplying by 3 for worst case scenario (all chars escaped)
   */
  char line[MP * 3 + 5];

  int restore_request = optind - 1;

  for (; restore_request < argc; restore_request++)
  {
    if ((func_error = bufchk (argv[restore_request], PATH_MAX)))
      return EXIT_BUF_ERR;

    file.base_name = basename (argv[restore_request]);

/**
 * The 2 code blocks below address
 * restoring files with only the basename #14
 */
    if ((strcmp (file.base_name, argv[restore_request]) == 0) &&
          file_not_found (file.base_name))
    {
      fprintf (stdout, "Searching using only the basename...\n");
      unsigned short ctr = 0;

      while (ctr < waste_dirs_total)
      {
        char *possibly_in_path[1];

        possibly_in_path[0] = waste[ctr].files;

        strcat (possibly_in_path[0], argv[restore_request]);

        Restore (1, possibly_in_path, 1, time_str_appended, waste,
                  waste_dirs_total);

        ctr++;
      }

      fprintf (stdout, "search complete\n");
      continue;
    }

    if (file_not_found (argv[restore_request]))
      fprintf (stderr, "%s not found\n", argv[restore_request]);

    else
    {
      strcpy (file.relative_path, argv[restore_request]);

      truncate_str (file.relative_path, strlen (file.base_name));

      strcpy (file.info, file.relative_path);
      strcat (file.info, "../info/");
      strcat (file.info, file.base_name);
      strcat (file.info, DOT_TRASHINFO);

      /**
       * No open files yet, so just return if bufchk fails
       */
      if ((func_error = bufchk (file.info, MP)))
        return func_error;

      if (file_not_found (file.info))
      {
        printf ("Error: no info file found for %s\n", argv[restore_request]);
        break;
      }

      else
      {
        FILE *fp;

        fp = fopen (file.info, "r");

        if (fp == NULL)
        {
          open_err (file.info, __func__);
          break;
        }

        else if (fgets (line, sizeof (line), fp ) != NULL)
        {
          /**
           * Not using the "[Trash Info]" line, but reading the file
           * sequentially
           */

          if (strncmp (line, "[Trash Info]", 12) == 0)
          {}

          else
          {
            fprintf (stderr, "Error: trashinfo file format not correct\n");
            fprintf (stderr, "(Line 1): %s\n", file.info);

            close_file (fp, file.info, __func__);

            break;
          }

          /** adding 5 for the 'Path=' preceding the path. */
          if (fgets (line, MP * 3 + 5, fp) != NULL)
          {
            char *tokenPtr;

            tokenPtr = strtok (line, "=");
            tokenPtr = strtok (NULL, "=");

            /**
             * tokenPtr now equals the escaped absolute path from the info file
             */
            unescape_url (tokenPtr, file.dest, MP);
            tokenPtr = NULL;
            trim (file.dest);

            close_file (fp, file.info, __func__);

          }

          else
          {
            printf ("error on line 2 in %s\n", file.info);

            close_file (fp, file.info, __func__);

            break;
          }

          /* Check for duplicate filename
           */
          if (!file_not_found (file.dest))
          {
            bufchk_string_op (CONCAT, file.dest, time_str_appended, MP);

            if (verbose == 1)
              fprintf (stdout,
                       "Duplicate filename at destination - appending time string...\n");
          }

          char parent_dir[MP];

          bufchk_string_op (COPY, parent_dir, file.dest, MP);

          truncate_str (parent_dir, strlen (basename (file.dest)));

          if (file_not_found (parent_dir))
            make_dir (parent_dir);

          if (!rename (argv[restore_request], file.dest))
          {
            printf ("+'%s' -> '%s'\n", argv[restore_request], file.dest);
            if (remove (file.info) != 0)
              fprintf (stderr, "error removing info file: '%s'\n", file.info);

            else
              if (verbose)
                printf ("-%s\n", file.info);
          }

          else
            fprintf (stderr, "Restore failed: %s\n", file.dest);
        }

        else
        {
          fprintf (stderr, "Error: Able to open %s but encountered an error\n",
              file.info);
          return 1;
        }
      }
    }
  }

  return 0;
}

/*
 * restore_select()
 *
 * Displays files that can be restored, user can select a file by
 * entering the corresponding number
 *
 * FIXME: This function needs to be re-worked
 */
void
restore_select (struct waste_containers *waste, char *time_str_appended,
                const int waste_dirs_total)
{
  struct stat st;
  struct dirent *entry;
  char path_to_file[MP];

  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char
   */
  char *destiny[1];

  unsigned count = 0;
  int w = 0;
  char input[10];
  char c;
  unsigned char char_count = 0;
  short choice = 0;

  while (w < waste_dirs_total)
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
        bufchk_string_op (COPY, path_to_file, waste[w].files, MP);

        /* Not yet sure if 'trim' is needed yet; using it
         *  until I get smarter
         */
        trim (entry->d_name);

        bufchk_string_op (CONCAT, path_to_file, entry->d_name, MP);
        trim (path_to_file);
        lstat (path_to_file, &st);
      }

      if (count == choice)
      {
        destiny[0] = path_to_file;
        printf ("\n");

        Restore (1, destiny, 1, time_str_appended, waste, waste_dirs_total);
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
      printf ("Input number to restore, 'enter' for next WASTE folder, 'q' to quit) ");
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

    /* If user selects 'q' to abort
     */
    if (c == 'q')
    {
      printf ("\n");
      return;
    }

    if (choice == 0)
      w++;
  }
}

void
undo_last_rmw (const char *HOMEDIR, char *time_str_appended,
               struct waste_containers *waste, const int waste_dirs_total)
{
  FILE *undo_file_ptr;
  char undo_path[MP];
  char line[MP];

  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char */
  char *destiny[1];
  bufchk_string_op (COPY, undo_path, HOMEDIR, MP);
  bufchk_string_op (CONCAT, undo_path, UNDO_FILE, MP);

  undo_file_ptr = fopen (undo_path, "r");

  if (undo_file_ptr != NULL)
  {}

  else
  {
    open_err (undo_path, __func__);

    return;
  }

  while (fgets (line, MP - 1, undo_file_ptr) != NULL)
  {
    trim (line);
    destiny[0] = line;

    Restore (1, destiny, 1, time_str_appended, waste, waste_dirs_total);
  }

  close_file (undo_file_ptr, undo_path, __func__);
}

/**
 * getch() and getche()
 * AUTHOR: zobayer
 *
 * reads from keypress, doesn't echo
 */
int
getch (void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr (STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar ();
  tcsetattr (STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}

/* reads from keypress, echoes */
int
getche (void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr (STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON);
  tcsetattr (STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar ();
  tcsetattr (STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}
