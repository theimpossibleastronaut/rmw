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

#include "purging_rmw.h"

int
rmdir_recursive (char *path, short unsigned level)
{
  if (level == RMDIR_MAX_DEPTH)
    return MAX_DEPTH_REACHED;

  short unsigned status = 0;
  struct stat st;
  struct dirent *entry;
  DIR *dir;
  char dir_path[PATH_MAX + 1];

  dir = opendir (path);

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

    if (st.st_mode & S_IWUSR)
    {
      if (!S_ISDIR (st.st_mode))
        remove (dir_path);

      else
      {
/*
 * if this function returns a 1 when it is called, that means on
 * a previous recursion, it encountered a file that wasn't writeable,
 * and therefore the top-level directory can't be deleted. It will skip
 * all future recursions, and also skip looking at files or directories
 * in the parent directories.
 *
 * In order for the purge to actually happen, the user must make sure all
 * files and dirs under the top level target directory are writeable
 */
        status = rmdir_recursive (dir_path, ++level);
        level--;

        switch (status)
        {

        case NOT_WRITEABLE:
          closedir (dir);
          return NOT_WRITEABLE;
          break;
        case MAX_DEPTH_REACHED:
          closedir (dir);
          return MAX_DEPTH_REACHED;
          break;
        }
      }
    }

    else
    {
      fprintf (stderr, "\nPermission denied while deleting\n");
      fprintf (stderr, "%s\n", dir_path);
      closedir (dir);
      return NOT_WRITEABLE;
    }
  }

  status = (closedir (dir));
  if (status)
    fprintf (stderr, "!-Error %d in rmdir_recursive()\n", status);

  if (level != 1)
  {
    status = rmdir (path);
    if (status)
      fprintf (stderr, "!-Error %d in rmdir_recursive()\n", status);
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
is_time_to_purge (bool force)
{
  char *HOMEDIR = getenv ("HOME");
  char file_lastpurge[MP];
  strcpy (file_lastpurge, HOMEDIR);
  strcat (file_lastpurge, PURGE_DAY_FILE);

  if (bufchk (file_lastpurge, MP))
    return EXIT_BUF_ERR;

  char today_dd[3];
  get_time_string (today_dd, 3, "%d");

  FILE *fp;

  if (!file_not_found (file_lastpurge))
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
      fprintf (stderr, "Error: while getting line from %s\n", file_lastpurge);
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
        char *time_now)
{
  if (purge_after > UINT_MAX)
  {
    printf ("purge_after can't be greater than %u\n", USHRT_MAX);
    return 1;
  }

  short status = 0;

  bool cmd_empty = 0;

  if (getenv("RMWTRASH") != NULL)
    cmd_empty = strcmp (getenv ("RMWTRASH"), "empty") ? 0 : 1;

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;

  char purgeFile[MP];

  struct tm tmPtr, tm_then;
  time_t now;
  time_t then;

  strptime (time_now, "%Y-%m-%dT%H:%M:%S", &tmPtr);
  now = mktime (&tmPtr);

  printf ("\nPurging files (purge_after = %u) ...\n", purge_after);

  struct dirent *entry;

  /**
   *  Read each Waste info directory
   */
  short ctr = START_WASTE_COUNTER;

  while (strcmp (waste[++ctr].parent, "NULL") != 0)
  {
    DIR *dir = opendir (waste[ctr].info);

    /**
     *  Read each file/dir in Waste directory
     */
    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      bufchk (entry->d_name, MP);

      FILE *info_file_ptr;

      char entry_path[MP];
      char trashinfo_line[MP + 5];

      char *tokenPtr;

      trim (entry->d_name);
      bufchk_string_op (COPY, entry_path, waste[ctr].info, MP);
      bufchk_string_op (CONCAT, entry_path, entry->d_name, MP);

      info_file_ptr = fopen (entry_path, "r");
      if (info_file_ptr != NULL)
      {
        /**
         * unused  and unneeded Trash Info line.
         * retrieved but not used.
         * Check to see if it's really a .trashinfo file
         */
        if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr) == NULL)
          continue;

        if (strncmp (trashinfo_line, "[Trash Info]", 12) != 0)
        {
          fprintf (stderr, "Info file error; format not correct (Line 1)\n");
          continue;
        }

        /** The second line is unneeded at this point
         * But check to see if there's a Path= statement to help ensure
         * that it's a properly formatted .trashinfo file
         */
        if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr) == NULL)
          continue;

        if (strncmp (trashinfo_line, "Path=", 5) != 0)
        {
          fprintf (stderr,
                   "Info file error; format not correct (Line 2) : %s\n",
                   entry_path);
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
          fprintf (stderr, "Info file error; format not correct (Line 3)\n");
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

      bool success = 0;

      if (then + (86400 * purge_after) <= now || cmd_empty)
      {
        success = 0;

        strcpy (purgeFile, waste[ctr].files);

        char temp[MP];
        strcpy (temp, entry->d_name);
        truncate_str (temp, strlen (DOT_TRASHINFO));
        strcat (purgeFile, temp);

        lstat (purgeFile, &st);

        if (S_ISDIR (st.st_mode))
        {
          status = rmdir_recursive (purgeFile, 1);

          switch (status)
          {

          case NOT_WRITEABLE:
            fprintf (stderr,
                     "Directory not purged - still contains files\n");
            fprintf (stderr, "%s\n", purgeFile);
            fprintf (stderr, "(check owner/write permissions)\n");
            dirs_containing_files_ctr++;
            break;

          case MAX_DEPTH_REACHED:
            fprintf (stderr, "Maximum depth of %u reached, skipping\n",
                     RMDIR_MAX_DEPTH);
            fprintf (stderr, "%s\n", purgeFile);
            max_depth_reached_ctr++;
            break;

          case 0:
            status = rmdir (purgeFile);
            if (!status)
              success = 1;

            else
            {
              fprintf (stderr, "Error: while removing %s\n", purgeFile);
              perror (__func__);
            }
            break;

          default:
            fprintf (stderr, "Error: while removing %s\n", purgeFile);
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
            fprintf (stderr, "Error: while removing %s\n", purgeFile);
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
              fprintf (stdout, "-%s\n", purgeFile);
          }
          else
          {
            fprintf (stderr, "Error: while removing %s\n", entry_path);
            perror (__func__);
          }
        }

      }
    }

    closedir (dir);

  }

  if (max_depth_reached_ctr)
    fprintf (stdout, "%d directories skipped (RMDIR_MAX_DEPTH reached)\n",
        max_depth_reached_ctr);

  if (dirs_containing_files_ctr)
    fprintf (stdout, "%d directories skipped (contained non-writeable files)\n",
        dirs_containing_files_ctr);

  if (purge_ctr == 1)
    fprintf (stdout, "%d file purged\n", purge_ctr);

  else
    fprintf (stdout, "%d files purged\n", purge_ctr);

  return 0;

}

short
orphan_maint(struct waste_containers *waste,
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

  while (strcmp (waste[++ctr].parent, "NULL") != 0)
  {
    struct dirent *entry;
    DIR *files;
    files = opendir (waste[ctr].files);

    while ((entry = readdir (files)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0 ||
          strcmp (entry->d_name, "..") == 0)
        continue;

      strcpy (file.base_name, basename (entry->d_name));

      strcpy (path_to_trashinfo, waste[ctr].info);
      strcat (path_to_trashinfo, file.base_name);
      strcat (path_to_trashinfo, DOT_TRASHINFO);

      if (!file_not_found (path_to_trashinfo))
        continue;

      strcpy (file.real_path, waste[ctr].parent);
      strcat (file.real_path, "/orphans/"); /* destination if restored */
      strcat (file.real_path, file.base_name);

      short ok = 0;
      ok = create_trashinfo (file, waste, time_now, time_str_appended, ctr);
      if (ok == 0)
        fprintf (stdout, "Created %s\n", path_to_trashinfo);
      else
        fprintf (stdout, "Error: while creating %s\n", path_to_trashinfo);

    }
    closedir (files);

  }

  return 0;
}





