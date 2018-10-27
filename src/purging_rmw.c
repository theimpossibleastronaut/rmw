/*
 * purging_rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2017  Andy Alt (andy400-dev@yahoo.com)
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

    bufchk (path, MP);
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

    if (force == 2 && ~st.st_mode&S_IWUSR)
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

int
purge (const short purge_after, const st_waste *waste_curr,
       char *time_now, const ushort force, const char *HOMEDIR)
{
  short status = 0;

  bool cmd_empty = 0;

  if (getenv ("RMWTRASH") != NULL)
    cmd_empty = strcmp (getenv ("RMWTRASH"), "empty") ? 0 : 1;

  if(cmd_empty)
    printf (_("\nPurging all files in waste folders ...\n"));
  else
    printf (_("\nPurging files based on number of days in the waste folders (%u) ...\n"), purge_after);

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;

  struct tm tmPtr, tm_then;
  time_t now;
  time_t then = 0;

  strptime (time_now, "%Y-%m-%dT%H:%M:%S", &tmPtr);
  now = mktime (&tmPtr);

  while (waste_curr != NULL)
  {
    static struct dirent *entry;
    DIR *dir = opendir (waste_curr->info);
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
      sprintf (entry_path, "%s%s", waste_curr->info, entry->d_name);

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
          bool passed = 0;
        /**
         * unused  and unneeded Trash Info line.
         * retrieved but not used.
         * Check to see if it's really a .trashinfo file
         */
          if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr) != NULL)
          {
            if (strncmp (trashinfo_line, "[Trash Info]", 12) == 0)
              if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr) != NULL)
                if (strncmp (trashinfo_line, "Path=", 5) == 0)
                   if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr) != NULL)
                   {
                      bufchk (trashinfo_line, 40);
                      trim (trashinfo_line);
                      if (strncmp (trashinfo_line, "DeletionDate=", 13) == 0 && strlen (trashinfo_line) == 32)
                      passed = 1;
                    }
          }

          close_file (info_file_ptr, entry_path, __func__);

          if (! passed)
          {
            display_dot_trashinfo_error (entry_path);
            continue;
          }
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

        strcpy (purgeFile, waste_curr->files);

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

    waste_curr = waste_curr->next_node;
  }

  if (max_depth_reached_ctr)
    printf (_("%d directories skipped (RMDIR_MAX_DEPTH reached)\n"),
             max_depth_reached_ctr);

  if (dirs_containing_files_ctr)
    printf (
             _("%d directories skipped (contains read-only files)\n"),
             dirs_containing_files_ctr);

  printf (ngettext("%d file purged" , "%d files purged", purge_ctr), purge_ctr);
  printf ("\n");

  return 0;

}

short
orphan_maint (st_waste *waste_curr,
              char *time_now, char *time_str_appended)
{
  struct rmw_target file;

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  file.is_duplicate = 0;

  char path_to_trashinfo[MP];

  int status;

  while (waste_curr != NULL)
  {
    struct dirent *entry;
    DIR *files;
    files = opendir (waste_curr->files);
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

      bufchk (basename (entry->d_name), MP);
      strcpy (file.base_name, basename (entry->d_name));

      sprintf (path_to_trashinfo, "%s%s%s", waste_curr->info, file.base_name, DOT_TRASHINFO);

      if (!exists (path_to_trashinfo))
        continue;

      /* destination if restored */
      sprintf (file.real_path, "%s%s%s", waste_curr->parent, "/orphans/", file.base_name);

      short ok = 0;
      ok = create_trashinfo (file, waste_curr, time_now, time_str_appended);
      if (ok == 0)
      /* TRANSLATORS:  "created" refers to a file  */
        printf (_("Created %s\n"), path_to_trashinfo);
      else
        printf (_("  :Error: while creating %s\n"), path_to_trashinfo);

    }
    status = closedir (files);
    if (status)
      perror ("rmdir_recursive -> closedir");

    waste_curr = waste_curr->next_node;
  }

  return 0;
}
