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

static char*
human_readable_size (off_t size) 
{
  /* "xxxx.y GiB" - 10 chars + '\0' */
  static char buffer[12];
  
  /* Store only the first letter; we add "iB" later during snprintf(). */
  const char prefix[] = {'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
  short power = -1;
  
  short remainder;
  while(size >= 1024)
  {
    remainder = size % 1024;
    size /= 1024;
    
    ++power;
  }
  
  if(power >= 0)
    snprintf(buffer, sizeof(buffer), "%ld.%hd %ciB", (long)size, (remainder * 10) / 1024, prefix[power]);
  else
    snprintf(buffer, sizeof(buffer), "%ld B", (long)size);
  
  return buffer;
}

/**
 * FIXME: This apparently needs re-working too. I'm sure it could be
 * written more efficiently
 */
int
Restore (char *argv, char *time_str_appended, struct waste_containers *waste)
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

  if ((func_error = bufchk (argv, PATH_MAX)))
    return EXIT_BUF_ERR;

  file.base_name = basename (argv);

/**
 * The 2 code blocks below address
 * restoring files with only the basename #14
 */
  if ((strcmp (file.base_name, argv) == 0) &&
        exists (file.base_name))
  {
    fprintf (stdout, "Searching using only the basename...\n");

    short ctr = START_WASTE_COUNTER;

    while (strcmp (waste[++ctr].parent, "NULL") != 0)
    {
      char *possibly_in_path;

      possibly_in_path = waste[ctr].files;

      strcat (possibly_in_path, argv);

      Restore (possibly_in_path, time_str_appended, waste);
    }

    fprintf (stdout, "search complete\n");

    return 0;
  }

  if (!exists (argv))
  {
    strcpy (file.relative_path, argv);

    truncate_str (file.relative_path, strlen (file.base_name));

    sprintf (file.info, "%s%s%s%s", file.relative_path, "../info/", file.base_name, DOT_TRASHINFO);

#if DEBUG == 1
  printf ("Restore()/debug: %s\n", file.info);
#endif

      /**
       * No open files yet, so just return if bufchk fails
       */
    if ((func_error = bufchk (file.info, MP)))
      return func_error;

    FILE *fp;

    if ((fp = fopen (file.info, "r")) != NULL)
    {
      if (fgets (line, sizeof (line), fp ) != NULL)
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

          return 1;
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

          return 1;
        }

          /* Check for duplicate filename
           */
        if (!exists (file.dest))
        {
          bufchk_string_op (CONCAT, file.dest, time_str_appended, MP);

          if (verbose)
            fprintf (stdout,"\
Duplicate filename at destination - appending time string...\n");
        }

        char parent_dir[MP];

        bufchk_string_op (COPY, parent_dir, file.dest, MP);

        truncate_str (parent_dir, strlen (basename (file.dest)));

        if (exists (parent_dir))
          make_dir (parent_dir);

        if (!rename (argv, file.dest))
        {
          printf ("+'%s' -> '%s'\n", argv, file.dest);

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
    else
    {
      open_err (file.info, __func__);
      return 1;
    }
  }
  else
  {
    fprintf (stderr, "%s not found\n", argv);
    return 1;
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
restore_select (struct waste_containers *waste, char *time_str_appended)
{
  struct stat st;
  struct dirent *entry;
  char path_to_file[MP];

  unsigned count = 0;
  char input[10];
  char c;
  unsigned char char_count = 0;
  short choice = 0;

  /*
   * ctr increments at the end of the loop, if choice hasn't been made,
   * so not using START_WASTE_COUNTER here
   */

  short ctr = 0;

  while (strcmp (waste[ctr].parent, "NULL") != 0)
  {
    DIR *dir = opendir (waste[ctr].files);
    count = 0;

    if (!choice)
      printf ("\t>-- %s --<\n", waste[ctr].files);

    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".")  || !strcmp (entry->d_name, ".."))
        continue;

      count++;

      if (count == choice || choice == 0)
      {
        bufchk_string_op (COPY, path_to_file, waste[ctr].files, MP);

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
        printf ("\n");

        Restore (path_to_file, time_str_appended, waste);
        break;
      }

      if (!choice)
      {
        printf ("%3d. %s [%s]", count, entry->d_name, human_readable_size(st.st_size));

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
      ctr++;
  }
}

void
undo_last_rmw (char *time_str_appended, struct waste_containers *waste)
{
  FILE *undo_file_ptr;
  char undo_path[MP];
  char line[MP];

  #ifndef WIN32
  char *HOMEDIR = getenv ("HOME");
  #else
  char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif
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

  int err_ct = 0;

  while (fgets (line, MP - 1, undo_file_ptr) != NULL)
  {
    trim (line);
    err_ct += Restore (line, time_str_appended, waste);
  }

  close_file (undo_file_ptr, undo_path, __func__);

  if (err_ct == 0)
  {
    if (remove (undo_path))
    {
      fprintf (stderr, "Warning: failed to remove %s\n", undo_path);
      perror (__func__);
    }

    return;
  }

  fprintf (stderr, "Warning: Restore() returned errors\n");

  return;
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
