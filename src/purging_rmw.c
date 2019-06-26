/*!
 * @file purging_rmw.c
 * @brief functions related to the purge features of rmw
 */
/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
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

static unsigned int deleted_files_ctr = 0;
static unsigned int deleted_dirs_ctr = 0;
static off_t bytes_freed = 0;

/*!
 * remove dirs recursively, primarily used by @ref purge()
 * @param[in] path the directory to be removed
 * @param[out] level keeps track of the number of times recursion has happened
 * @return error number
 */
static int
rmdir_recursive (char *path, short unsigned level)
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
    msg_err_open_dir (path, __func__, __LINE__);

  while ((entry = readdir (dir)) != NULL)
  {
    if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
      continue;

    bufchk (path, MP);
    snprintf (dir_path, MP, "%s", path);

    short pathLen = strlen (dir_path);
    if (dir_path[pathLen - 1] != '/')
    {
      dir_path[pathLen] = '/';
      pathLen++;
      dir_path[pathLen] = '\0';
    }

    strcat (dir_path, entry->d_name);

    lstat (dir_path, &st);
    extern const ushort force;
    if (force == 2 && ~st.st_mode & S_IWUSR)
    {
      if (!chmod (dir_path, 00700))
      {
        /* Now that the mode has changed, lstat must be run again */
        lstat (dir_path, &st);
      }
      else
      {
        print_msg_error ();
        printf (_("while changing permissions of %s\n"), path);
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
        if (status == 0)
        {
          deleted_files_ctr++;
          bytes_freed += st.st_size;
        }
        else
          perror ("rmdir_recursive -> remove");
      }
      else
      {

        status = rmdir_recursive (dir_path, ++level);
        level--;

        switch (status)
        {

        case NOT_WRITEABLE:
          if (closedir (dir))
            msg_err_close_dir (path, __func__, __LINE__);
          return NOT_WRITEABLE;
          break;
        case MAX_DEPTH_REACHED:
          if (closedir (dir))
            msg_err_close_dir (path, __func__, __LINE__);
          return MAX_DEPTH_REACHED;
          break;
        }
      }
    }

    else
    {
      printf ("\nPermission denied while deleting\n");
      printf ("%s\n", dir_path);

      if (closedir (dir))
        msg_err_close_dir (path, __func__, __LINE__);

      return NOT_WRITEABLE;
    }
  }

  if (closedir (dir))
    msg_err_close_dir (path, __func__, __LINE__);

  if (level > 1)
  {
    status = rmdir (path);
    if (status == 0)
      deleted_dirs_ctr++;
    else
      perror ("rmdir_recursive -> rmdir");
  }

  return status;
}

/*!
 * Get the time a file was rmw'ed by reading the corresponding trashinfo
 * file. Called from @ref purge()
 * @param[in] entry directory entry
 * @param[in] entry_path path to directory entry
 * @return time_t value
 */
static time_t
get_then_time(struct dirent *entry, const char *entry_path)
{
  bufchk (entry->d_name, MP);

  FILE *info_file_ptr;
  char trashinfo_line[MP + 5];
  char *tokenPtr;
  struct tm tm_then;
  time_t then = 0;

  info_file_ptr = fopen (entry_path, "r");

  if (info_file_ptr != NULL)
  {
    bool passed = 0;
    /*
    * unused  and unneeded Trash Info line.
    * retrieved but not used.
    * Check to see if it's really a .trashinfo file
    */
    if (fgets (trashinfo_line, sizeof (trashinfo_line), info_file_ptr)
        != NULL)
    {
      if (strncmp (trashinfo_line, "[Trash Info]", 12) == 0)
        if (fgets
            (trashinfo_line, sizeof (trashinfo_line),
             info_file_ptr) != NULL)
          if (strncmp (trashinfo_line, "Path=", 5) == 0)
            if (fgets
                (trashinfo_line, sizeof (trashinfo_line),
                 info_file_ptr) != NULL)
            {
              bufchk (trashinfo_line, 40);
              trim_white_space (trashinfo_line);
              if (strncmp (trashinfo_line, "DeletionDate=", 13) == 0
                  && strlen (trashinfo_line) == 32)
                passed = 1;
            }
    }
    close_file (info_file_ptr, entry_path, __func__);

    if (!passed)
    {
      display_dot_trashinfo_error (entry_path);
      goto out;
    }
  }
  else
  {
    open_err (entry_path, __func__);
    goto out;
  }

  tokenPtr = strtok (trashinfo_line, "=");
  tokenPtr = strtok (NULL, "=");

  strptime (tokenPtr, "%Y-%m-%dT%H:%M:%S", &tm_then);
  then = mktime (&tm_then);
out:
  return then;
}

/*!
 * Purges files older than x number of days, unless purge_after is set to
 * 0 in the config file.
 * @param[in] waste_curr the linked list of waste folders
 * @return error number
 * @see is_time_to_purge
 * @see get_then_time
 */
int
purge (const st_waste * waste_curr)
{
  short status = 0;

  bool cmd_empty = 0;
  bool cmd_dry_run = 0;
  char *rmwtrash_env = getenv("RMWTRASH");

  if (rmwtrash_env != NULL)
  {
    cmd_empty = strcmp (rmwtrash_env, "empty") ? 0 : 1;
    cmd_dry_run = strcmp (rmwtrash_env, "dry-run") ? 0 : 1;
  }

  /* if dry-run was enabled, assume verbosity as well */
  if (cmd_dry_run)
    verbose = 1;

  extern const int purge_after;
  printf ("\n");
  if (cmd_empty)
    printf (_("Purging all files in waste folders ...\n"));
  else
    printf (_("Purging files based on number of days in the waste folders (%u) ...\n"),
            purge_after);

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;

  struct tm tmPtr;
  time_t now;
  time_t then = 0;

  extern const char *time_now;
  strptime (time_now, "%Y-%m-%dT%H:%M:%S", &tmPtr);
  now = mktime (&tmPtr);

  while (waste_curr != NULL)
  {
    static struct dirent *entry;
    DIR *dir = opendir (waste_curr->info);
    if (dir == NULL)
      msg_err_open_dir (waste_curr->info, __func__, __LINE__);

    /*
     *  Read each file in <WASTE>/info
     */
    while ((entry = readdir (dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      int req_len = multi_strlen (2, waste_curr->info, entry->d_name) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      char entry_path[MP];
      snprintf (entry_path, req_len, "%s%s", waste_curr->info, entry->d_name);

      /* skip this block if RMWTRASH=empty */
      if (!cmd_empty && !(then = get_then_time(entry, entry_path)))
          continue;

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
          if (!cmd_dry_run)
            status = rmdir_recursive (purgeFile, 1);
          else
          {
            /* Not much choice but to
             * assume there would not be an error if the attempt were actually made */
            status = 0;
          }

          switch (status)
          {
          case NOT_WRITEABLE:
            print_msg_warn ();
            printf (_("Directory not purged - still contains files\n"));
            printf ("%s\n", purgeFile);
            printf (_("(check owner/write permissions)\n"));
            dirs_containing_files_ctr++;
            break;

          case MAX_DEPTH_REACHED:
            print_msg_warn ();
            /* TRANSLATORS:  "depth" refers to the recursion depth in a
             * directory   */
            printf (_("Maximum depth of %u reached, skipping\n"),
                    RMDIR_MAX_DEPTH);
            printf ("%s\n", purgeFile);
            max_depth_reached_ctr++;
            break;

          case 0:
            if (!cmd_dry_run)
              status = rmdir (purgeFile);
            else
              status = 0;

            if (status == 0)
            {
              success = 1;
              deleted_dirs_ctr++;
              bytes_freed += st.st_size;
            }
            else
            {
              print_msg_error ();
              /* TRANSLATORS:  "removing" refers to a file or folder  */
              printf (_("while removing %s\n"), purgeFile);
              perror (__func__);
            }
            break;

          default:
            print_msg_error ();
            printf (_("while removing %s\n"), purgeFile);
            perror (__func__);
            break;
          }

        }

        else
        {
          if (!cmd_dry_run)
            status = remove (purgeFile);
          else
            status = 0;

          if (status == 0)
          {
            success = 1;
            deleted_files_ctr++;
            bytes_freed += st.st_size;
          }
          else
          {
            print_msg_error ();
            printf (_("while removing %s\n"), purgeFile);
            perror (__func__);
            success = 0;
          }
        }

        if (success)
        {
          /* the info file. (This var name needs
           *changing) */
          if (!cmd_dry_run)
            status = remove (entry_path);
          else
            status = 0;

          if (!status)
          {
            purge_ctr++;
            if (verbose)
              printf ("-%s\n", purgeFile);
          }
          else
          {
            print_msg_error ();
            printf (_("while removing %s\n"), entry_path);
            perror (__func__);
          }
        }

      }
    }

    if (closedir (dir))
      msg_err_close_dir (waste_curr->info, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  if (max_depth_reached_ctr)
    printf (_("%d directories skipped (RMDIR_MAX_DEPTH reached)\n"),
            max_depth_reached_ctr);

  if (dirs_containing_files_ctr)
    printf (_("%d directories skipped (contains read-only files)\n"),
            dirs_containing_files_ctr);

  printf (ngettext ("%d file purged", "%d files purged", purge_ctr),
          purge_ctr);
  printf ("\n");
  printf (ngettext
          ("(%d file deleted)", "(%d files deleted)", deleted_files_ctr),
          deleted_files_ctr);
  printf ("\n");
  printf (ngettext
          ("(%d directory deleted)", "(%d directories deleted)",
           deleted_dirs_ctr), deleted_dirs_ctr);
  printf ("\n");
  printf (ngettext ("%s freed", "%s freed", bytes_freed),
          human_readable_size (bytes_freed));
  printf ("\n");

  return 0;

}

short
orphan_maint (st_waste * waste_curr)
{
  rmw_target *file = (rmw_target *) malloc (sizeof (rmw_target));
  chk_malloc (file, __func__, __LINE__);

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  file->is_duplicate = 0;

  char path_to_trashinfo[MP];
  int orphan_ctr = 0;
  while (waste_curr != NULL)
  {
    struct dirent *entry;
    DIR *files;
    files = opendir (waste_curr->files);
    if (files == NULL)
      msg_err_open_dir (waste_curr->files, __func__, __LINE__);

    while ((entry = readdir (files)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0 ||
          strcmp (entry->d_name, "..") == 0)
        continue;

      bufchk (basename (entry->d_name), MP);
      strcpy (file->base_name, basename (entry->d_name));

      int req_len = multi_strlen (3, waste_curr->info, file->base_name,
               DOT_TRASHINFO) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      snprintf (path_to_trashinfo, req_len, "%s%s%s", waste_curr->info, file->base_name,
               DOT_TRASHINFO);

      if (exists (path_to_trashinfo))
        continue;

      /* destination if restored */
      req_len = multi_strlen(3, waste_curr->parent, "/orphans/", file->base_name) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      snprintf (file->real_path, req_len, "%s%s%s", waste_curr->parent, "/orphans/",
               file->base_name);

      if (!create_trashinfo (file, waste_curr))
      {
        /* TRANSLATORS:  "created" refers to a file  */
        printf (_("Created %s\n"), path_to_trashinfo);
        orphan_ctr++;
      }
      else
      {
        print_msg_error ();
        printf (_("while creating %s\n"), path_to_trashinfo);
      }

    }
    if (closedir (files))
      msg_err_close_dir (waste_curr->files, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  printf ("%d orphans found\n", orphan_ctr);

  free (file);
  file = NULL;

  return 0;
}
