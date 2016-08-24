/*
 * purging_rmw.c
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
      fprintf (stderr, "!-Permission denied while deleting\n");
      fprintf (stderr, "\t%s\n", dir_path);
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
 * writes the day of the last purge
 * FIXME: function name needs changing
 * variable names need changing
 */
bool
purgeD (const char *HOMEDIR)
{
  FILE *fp;

  char purgeDpath[MP];

  strcpy (purgeDpath, HOMEDIR);
  buf_check_with_strop (purgeDpath, PURGE_DAY_FILE, CAT);

  char lastDay[3];
  char nowD[3];

  /** a check for buffer overflow is in get_time_string() */
  get_time_string (nowD, 3, "%d");

  /**
   * Already been checked for a buffer overflow, just want to add a NULL
   * terminator, in case something got hosed. (trim() will add the NULL
   */
  trim (purgeDpath);

  if (!file_not_found (purgeDpath))
  {
    fp = fopen (purgeDpath, "r");
    /*
     * Need to do some error checking upon opening and closing
     */
    fgets (lastDay, 3, fp);
    buf_check (lastDay, 3);
    trim (lastDay);
    fclose (fp);

    if (!strcmp (nowD, lastDay))
      // Same day
      return 0;

    /** Days differ, write the new day. */
    else
    {
      fp = fopen (purgeDpath, "w");
      if (fp != NULL)
      {
        fprintf (fp, "%s\n", nowD);
        fclose (fp);
        return 1;
      }
      else
      {
        fprintf (stderr, "error %d writing to %s\n", errno, purgeDpath);
        exit (1);
      }
    }

  }
  else
  {
    /**
     * Create file if it doesn't exist
     */
    fp = fopen (purgeDpath, "w");

    if (fp != NULL)
    {
      fprintf (fp, "%s\n", nowD);
      fclose (fp);
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
      fprintf (stderr, "Fatal: Error %d creating %s\n", errno, purgeDpath);
      exit (1);
    }
  }
}

int
purge (const short *pa, const struct waste_containers *waste, char *time_now,
       const int wdt)
{
  if (*pa > UINT_MAX)
  {
    printf ("purge_after can't be greater than %u\n", USHRT_MAX);
    exit (1);
  }

  short status = 0;

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

  printf ("\nPurging files older than %u days...\n", *pa);

  int p = 0;
  struct dirent *entry;

  /**
   *  Read each Waste info directory
   */
  while (p < wdt && p < WASTENUM_MAX)
  {
    DIR *dir = opendir (waste[p].info);

    /**
     *  Read each file/dir in Waste directory
     */
    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      buf_check (entry->d_name, MP);

      FILE *info_file_ptr;
      const short timeLine = 40;
      char entry_path[MP];
      char infoLine[MP + 5];

      char *tokenPtr;

      trim (entry->d_name);
      buf_check_with_strop (entry_path, waste[p].info, CPY);
      buf_check_with_strop (entry_path, entry->d_name, CAT);

      info_file_ptr = fopen (entry_path, "r");
      if (info_file_ptr != NULL)
      {
        /**
         * unused  and unneeded Trash Info line.
         * retrieved but not used.
         */
        fgets (infoLine, 14, info_file_ptr);

        if (strncmp (infoLine, "[Trash Info]", 12) != 0)
        {
          fprintf (stderr, "Info file error; format not correct (Line 1)\n");
          exit (1);
        }

        /** The second line is unneeded at this point */
        fgets (infoLine, MP + 5, info_file_ptr);

        if (strncmp (infoLine, "Path=", 5) != 0)
        {
          fprintf (stderr,
                   "Info file error; format not correct (Line 2) : %s\n",
                   entry_path);
          exit (1);
        }

        /** The third line is needed for the deletion time */
        fgets (infoLine, timeLine, info_file_ptr);
        buf_check (infoLine, 40);
        trim (infoLine);
        if (strncmp (infoLine, "DeletionDate=", 13) != 0
            || strlen (infoLine) != 32)
        {
          fprintf (stderr, "Info file error; format not correct (Line 3)\n");

         /**
          * This exit() is related to issue #8
          * https://github.com/andy5995/rmw/issues/8
          */
          exit (1);
        }

        if (fclose (info_file_ptr) == EOF)
        {
          fprintf (stderr, "Error: while closing %s\n", entry_path);
          perror ("purge()");
        }
      }

      else
      {
        fprintf (stderr, "Fatal: Error %d while opening %s\n", errno, entry_path);

       /**
        * This exit() is related to issue #8
        * https://github.com/andy5995/rmw/issues/8
        */
        exit (1);
      }

      tokenPtr = strtok (infoLine, "=");
      tokenPtr = strtok (NULL, "=");

      strptime (tokenPtr, "%Y-%m-%dT%H:%M:%S", &tm_then);
      then = mktime (&tm_then);

      bool success = 0;

      if (then + (86400 * *pa) <= now)
      {
        success = 0;

        strcpy (purgeFile, waste[p].files);

        char temp[MP];
        strcpy (temp, entry->d_name);
        truncate_str (temp, strlen (DOT_TRASHINFO));
        strcat (purgeFile, temp);

        lstat (purgeFile, &st);
        int err = 0;
        if (S_ISDIR (st.st_mode))
        {
          status = rmdir_recursive (purgeFile, 1);

          switch (status)
          {

          case NOT_WRITEABLE:
            fprintf (stderr,
                     "!-Directory not purged - still contains files\n");
            fprintf (stderr, "\t%s\n", purgeFile);
            fprintf (stderr, "\t(check owner/write permissions)\n");
            dirs_containing_files_ctr++;
            break;

          case MAX_DEPTH_REACHED:
            fprintf (stderr, "!-Maximum depth of %u reached, skipping\n",
                     RMDIR_MAX_DEPTH);
            fprintf (stderr, "\t%s\n", purgeFile);
            max_depth_reached_ctr++;
            break;

          case 0:
            status = rmdir (purgeFile);
            if (!status)
              success = 1;

            else
            {
              fprintf (stderr, "Error: while removing %s\n", purgeFile);
              perror ("purge()");
            }
            break;

          default:
            fprintf (stderr, "Error: while removing %s\n", purgeFile);
            perror ("purge()");
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
            perror ("purge()");
            success = 0;
          }
        }


        if (success)
        {
          status = remove (entry_path);

          if (!status)
          {
            purge_ctr++;
            fprintf (stdout, "-%s\n", purgeFile);
          }
          else
          {
            fprintf (stderr, "Error: while removing %s\n", entry_path);
            perror ("purge()");
          }
        }

      }
    }

    closedir (dir);
    p++;
  }

  if (max_depth_reached_ctr)
    fprintf (stdout, "%d directories skipped (RMDIR_MAX_DEPTH reached)\n",
        max_depth_reached_ctr);

  if (dirs_containing_files_ctr)
    fprintf (stdout, "%d directories skipped (contained non-writeable files)\n",
        dirs_containing_files_ctr);

  printf ("%d files purged\n", purge_ctr);
  return 0;

}
