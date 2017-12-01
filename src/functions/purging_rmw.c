/*
 * purging_rmw.c
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

#include <sys/stat.h>

#include <time.h>
/* strptime prototype from time.h
 *
 * This gets rid of the warning "implicit declaration of function ‘strptime’"
 */
char *strptime (const char *__restrict __s,
                       const char *__restrict __fmt, struct tm *__tp);
#include <dirent.h>
#include <unistd.h> /* for rmdir() */

#include "purging_rmw.h"
#include "messages_rmw.h"
#include "strings_rmw.h"
#include "utils_rmw.h"
#include "trashinfo_rmw.h"

static int rmdir_recursive (char *path, short unsigned level, const ushort force)
{
  if (level == RMDIR_MAX_DEPTH)
    return MAX_DEPTH_REACHED;

  int status = 0;

  struct stat st;
  struct dirent *entry;
  DIR *dir;
  char dir_path[PATH_MAX + 1];

  dir = opendir (path);
  if (dir == NULL)
    {
      perror ("opendir");
      exit (EXIT_OPENDIR_FAILURE);
    }

  while ((entry = readdir (dir)) != NULL)
  {
    if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
      continue;

    strcpy (dir_path, path);

    short pathLen = strlen (dir_path);
    if (dir_path[pathLen - 1] != '/')
    {
      dir_path[pathLen] = '/';
      pathLen++;
      dir_path[pathLen] = '\0';
    }

    strcat (dir_path, entry->d_name);

    lstat (dir_path, &st);

    if (S_ISDIR (st.st_mode) && force == 2 && st.st_mode != S_IWUSR)
    {
      if (!chmod (dir_path, 00700))
      {
        /* Now that the mode has changed, lstat must be run again */
        lstat (dir_path, &st);
      }
      else
      {
        printf (_("  :Error: while changing permissions of %s\n"),
        path);
        perror ("chmod: ");
        printf ("\n");
        /* if permissions aren't changed, the directory is still
        * not writable. This error shouldn't really happen. I don't
        * know what to do next if that happens. Right now, the program
        * will continue as normal, with the warning message about
        * permissions
        */
      }
    }

    if (st.st_mode & S_IWUSR)
    {
      if (!S_ISDIR (st.st_mode))
      {
        status = remove (dir_path);
        if (status != 0)
          perror ("rmdir_recursive -> remove");
      }
      else
      {

        status = rmdir_recursive (dir_path, ++level, force);
        level--;

        switch (status)
        {

        case NOT_WRITEABLE:
          status = closedir (dir);
          if (status)
            perror ("rmdir_recursive -> closedir");
          return NOT_WRITEABLE;
          break;
        case MAX_DEPTH_REACHED:
          status = closedir (dir);
          if (status)
            perror ("rmdir_recursive -> closedir");
          return MAX_DEPTH_REACHED;
          break;
        }
      }
    }

    else
    {
      printf ("\nPermission denied while deleting\n");
      printf ("%s\n", dir_path);

      status = closedir (dir);
      if (status)
        perror ("rmdir_recusive -> closedir");

      return NOT_WRITEABLE;
    }
  }

  status = (closedir (dir));
  if (status)
    perror ("rmdir_recursive -> closedir");

  if (level > 1)
  {
    status = rmdir (path);
    if (status)
      perror ("rmdir_recursive -> rmdir");
  }

  return status;
}

/*
 * is_time_to_purge()
 *
 * Creates lastpurge file
 * checks to see if purge() was run today
 * if not, returns 1 and writes the day
 * to the lastpurge file.
 */
bool
is_time_to_purge (ushort force)
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
      return 0;
    }

    char last_purge_dd[3];

    if (fgets (last_purge_dd, sizeof (last_purge_dd), fp) == NULL)
    {
      printf ("Error: while getting line from %s\n", file_lastpurge);
      perror (__func__);
      return 0;
    }

    bufchk (last_purge_dd, 3);
    trim (last_purge_dd);

    close_file (fp, file_lastpurge, __func__);

    /** if these are the same, purge has already been run today
     */

    if (!strcmp (today_dd, last_purge_dd))
      return 0;

    /**
     * Days differ, but today's date won't be written to lastpurge
     * Instead, after the function returns, the user will get the
     * message that --force is needed
     */
    if (!force)
      return 1;

    /** Days differ, force used - write the new day. */


    fp = fopen (file_lastpurge, "w");

    if (fp != NULL)
    {
      fprintf (fp, "%s\n", today_dd);

      close_file (fp, file_lastpurge, __func__);
          /** If the only error is upon closing, and all the checks above
           * passed, we'll just continue. The error was printed to stderr
           * and the cause needs to be checked by the user or the
           * developer
           */

      return 1;
    }

    else
    {
      open_err (file_lastpurge, __func__);

      return 0;
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

      return 1;
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
      exit (1);
    }
  }
}

int
purge (const short purge_after, const struct waste_containers *waste,
       char *time_now, const ushort force)
{
  short status = 0;

  bool cmd_empty = 0;

  if (getenv ("RMWTRASH") != NULL)
    cmd_empty = strcmp (getenv ("RMWTRASH"), "empty") ? 0 : 1;

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;

  struct tm tmPtr, tm_then;
  time_t now;
  time_t then;

  strptime (time_now, "%Y-%m-%dT%H:%M:%S", &tmPtr);
  now = mktime (&tmPtr);

  printf (_("\nPurging files (purge_after = %u) ...\n"), purge_after);

  struct dirent *entry;

  /**
   *  Read each <WASTE>/info directory
   */
  short ctr = START_WASTE_COUNTER;

  while (strcmp (waste[++ctr].parent, "NULL") != 0)
  {
    DIR *dir = opendir (waste[ctr].info);
    if (dir == NULL)
    {
      perror ("purge -> opendir");
      exit (EXIT_OPENDIR_FAILURE);
    }

    /**
     *  Read each file in <WASTE>/info
     */
    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      char entry_path[MP];
      sprintf (entry_path, "%s%s", waste[ctr].info, entry->d_name);

      // printf ("%s\n", entry_path);

      if (!cmd_empty)           /* skip this block if RMWTRASH=empty */
      {
        bufchk (entry->d_name, MP);

        FILE *info_file_ptr;

        char trashinfo_line[MP + 5];

        char *tokenPtr;

        info_file_ptr = fopen (entry_path, "r");

        if (info_file_ptr != NULL)
        {
        /**
         * unused  and unneeded Trash Info line.
         * retrieved but not used.
         * Check to see if it's really a .trashinfo file
         */
          if (fgets (trashinfo_line, sizeof (trashinfo_line),
              info_file_ptr) == NULL)
            continue;

          if (strncmp (trashinfo_line, "[Trash Info]", 12) != 0)
          {
            display_dot_trashinfo_error (entry_path, 1);
            continue;
          }

        /** The second line is unneeded at this point
         * But check to see if there's a Path= statement to help ensure
         * that it's a properly formatted .trashinfo file
         */
          if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr)
              == NULL)
            continue;

          if (strncmp (trashinfo_line, "Path=", 5) != 0)
          {
            display_dot_trashinfo_error (entry_path, 2);
            continue;
          }

        /** The third line is needed for the deletion time
         */
          if (fgets (trashinfo_line, sizeof (trashinfo_line),
                     info_file_ptr) == NULL)
            continue;

          bufchk (trashinfo_line, 40);
          trim (trashinfo_line);

          if (strncmp (trashinfo_line, "DeletionDate=", 13) != 0
              || strlen (trashinfo_line) != 32)
          {
            display_dot_trashinfo_error (entry_path, 3);
            continue;
          }

          close_file (info_file_ptr, entry_path, __func__);
        }

        else
        {
          open_err (entry_path, __func__);
          continue;
        }

        tokenPtr = strtok (trashinfo_line, "=");
        tokenPtr = strtok (NULL, "=");

        strptime (tokenPtr, "%Y-%m-%dT%H:%M:%S", &tm_then);
        then = mktime (&tm_then);
      }

      if (then + (86400 * purge_after) <= now || cmd_empty)
      {
        bool success = 0;

        char purgeFile[MP];

        strcpy (purgeFile, waste[ctr].files);

        char temp[MP];
        strcpy (temp, entry->d_name);
        truncate_str (temp, strlen (DOT_TRASHINFO));    /* acquire the basename */

        strcat (purgeFile, temp);       /* path to file in <WASTE>/files */

        lstat (purgeFile, &st);

        if (S_ISDIR (st.st_mode))
        {
          status = rmdir_recursive (purgeFile, 1, force);
          switch (status)
          {
          case NOT_WRITEABLE:
            printf (_(" :warning: Directory not purged - still contains files\n"));
            printf ("%s\n", purgeFile);
            printf (_("(check owner/write permissions)\n"));
            dirs_containing_files_ctr++;
            break;

          case MAX_DEPTH_REACHED:
          /* TRANSLATORS:  "depth" refers to the recursion depth in a
           * directory   */
            printf (_(" :warning: Maximum depth of %u reached, skipping\n"),
                     RMDIR_MAX_DEPTH);
            printf ("%s\n", purgeFile);
            max_depth_reached_ctr++;
            break;

          case 0:
            status = rmdir (purgeFile);
            if (!status)
              success = 1;
            else
            {
              /* TRANSLATORS:  "removing" refers to a file or folder  */
              printf (_("  :Error: while removing %s\n"), purgeFile);
              perror (__func__);
            }
            break;

          default:
            printf (_("  :Error: while removing %s\n"), purgeFile);
            perror (__func__);
            break;
          }

        }

        else
        {
          status = remove (purgeFile);

          if (!status)
            success = 1;
          else
          {
            printf (_("  :Error: while removing %s\n"), purgeFile);
            perror (__func__);
            success = 0;
          }
        }

        if (success)
        {
          status = remove (entry_path);

          if (!status)
          {
            purge_ctr++;
            if (verbose)
              printf ("-%s\n", purgeFile);
          }
          else
          {
            printf (_("  :Error: while removing %s\n"), entry_path);
            perror (__func__);
          }
        }

      }
    }

    status = closedir (dir);
    if (status)
      perror ("purge -> closedir");

  }

  if (max_depth_reached_ctr)
    printf (_("%d directories skipped (RMDIR_MAX_DEPTH reached)\n"),
             max_depth_reached_ctr);

  if (dirs_containing_files_ctr)
    printf (
             _("%d directories skipped (contains read-only files)\n"),
             dirs_containing_files_ctr);

  printf (purge_ctr == 1 ? _("%d file purged") : _("%d files purged"),
          purge_ctr);
  printf ("\n");

  return 0;

}

short
orphan_maint (struct waste_containers *waste,
              char *time_now, char *time_str_appended)
{
  struct rmw_target file;

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  file.is_duplicate = 0;

  char path_to_trashinfo[MP];

  short ctr = START_WASTE_COUNTER;

  int status;

  while (strcmp (waste[++ctr].parent, "NULL") != 0)
  {
    struct dirent *entry;
    DIR *files;
    files = opendir (waste[ctr].files);
    if (files == NULL)
    {
      perror ("orphan_maint -> opendir");
      exit (EXIT_OPENDIR_FAILURE);
    }

    while ((entry = readdir (files)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0 ||
          strcmp (entry->d_name, "..") == 0)
        continue;

      strcpy (file.base_name, basename (entry->d_name));

      sprintf (path_to_trashinfo, "%s%s%s", waste[ctr].info, file.base_name, DOT_TRASHINFO);

      if (!exists (path_to_trashinfo))
        continue;

      /* destination if restored */
      sprintf (file.real_path, "%s%s%s", waste[ctr].parent, "/orphans/", file.base_name);

      short ok = 0;
      ok = create_trashinfo (file, waste, time_now, time_str_appended, ctr);
      if (ok == 0)
      /* TRANSLATORS:  "created" refers to a file  */
        printf (_("Created %s\n"), path_to_trashinfo);
      else
        printf (_("  :Error: while creating %s\n"), path_to_trashinfo);

    }
    status = closedir (files);
    if (status)
      perror ("rmdir_recursive -> closedir");
  }

  return 0;
}
