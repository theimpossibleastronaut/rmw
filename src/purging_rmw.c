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
rmdir_recursive (char *dirname, short unsigned level, const rmw_options * cli_user_options)
{
  if (level == RMDIR_MAX_DEPTH)
    return MAX_DEPTH_REACHED;

  int remove_result = 0;

  struct _dirname {
    char path[MP];
    DIR *ptr;
    struct dirent *st_entry_ptr;
  } st_dirname_properties;

  st_dirname_properties.ptr = opendir (dirname);
  if (st_dirname_properties.ptr == NULL)
    msg_err_open_dir (dirname, __func__, __LINE__);

  while ((st_dirname_properties.st_entry_ptr = readdir (st_dirname_properties.ptr)) != NULL)
  {
    if (!strcmp (st_dirname_properties.st_entry_ptr->d_name, ".") || !strcmp (st_dirname_properties.st_entry_ptr->d_name, ".."))
      continue;

    bufchk (dirname, MP);
    strncpy (st_dirname_properties.path, dirname, MP);

    int pathLen = strlen (st_dirname_properties.path);
    if (st_dirname_properties.path[pathLen - 1] != '/')
    {
      st_dirname_properties.path[pathLen] = '/';
      pathLen++;
      st_dirname_properties.path[pathLen] = '\0';
    }

    int req_len = multi_strlen (st_dirname_properties.path, st_dirname_properties.st_entry_ptr->d_name, NULL) + 1;
    bufchk_len (req_len, MP, __func__, __LINE__);
    strncat (st_dirname_properties.path, st_dirname_properties.st_entry_ptr->d_name, req_len);

    struct stat st;
    if (lstat (st_dirname_properties.path, &st))
      msg_err_lstat (__func__, __LINE__);
    int orig_dev = st.st_dev;
    int orig_inode = st.st_ino;

    if (cli_user_options->force >= 2 && ~st.st_mode & S_IWUSR) /* use >= 2 to protect against future changes */
    {
      if (!chmod (st_dirname_properties.path, 00700))
      {
        /* Now that the mode has changed, lstat must be run again */
        if (lstat (st_dirname_properties.path, &st))
          msg_err_lstat (__func__, __LINE__);
        orig_dev = st.st_dev;
        orig_inode = st.st_ino;
      }
      else
      {
        print_msg_error ();
        printf (_("while changing permissions of %s\n"), dirname);
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
        if (!is_modified (st_dirname_properties.path, orig_dev, orig_inode))
          remove_result = remove (st_dirname_properties.path);
        else
          remove_result = -1;

        if (remove_result == 0)
        {
          deleted_files_ctr++;
          bytes_freed += st.st_size;
        }
        else
          perror ("rmdir_recursive -> remove");
      }
      else
      {
        remove_result = rmdir_recursive (st_dirname_properties.path, ++level, cli_user_options);
        level--;

        switch (remove_result)
        {

        case NOT_WRITEABLE:
          if (closedir (st_dirname_properties.ptr))
            msg_err_close_dir (dirname, __func__, __LINE__);
          return NOT_WRITEABLE;
          break;
        case MAX_DEPTH_REACHED:
          if (closedir (st_dirname_properties.ptr))
            msg_err_close_dir (dirname, __func__, __LINE__);
          return MAX_DEPTH_REACHED;
          break;
        }
      }
    }

    else
    {
      printf ("\nPermission denied while deleting\n");
      printf ("%s\n", st_dirname_properties.path);

      if (closedir (st_dirname_properties.ptr))
        msg_err_close_dir (dirname, __func__, __LINE__);

      return NOT_WRITEABLE;
    }
  }

  if (closedir (st_dirname_properties.ptr))
    msg_err_close_dir (dirname, __func__, __LINE__);

  if (level > 1)
  {
    if ((remove_result = rmdir (dirname)) == 0)
      deleted_dirs_ctr++;
    else
      perror ("rmdir_recursive -> rmdir");
  }

  return remove_result;
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
purge (const st_waste * waste_curr, const rmw_options * cli_user_options, time_t time_t_now)
{
  extern const int purge_after;
  if (!purge_after)
  {
  /* TRANSLATORS:  "purging" refers to permanently deleting a file or a
   * directory  */
    printf (_("purging is disabled ('purge_after' is set to '0')\n\n"));

    /* purge_after is kind of a "fail-safe". If someone sets it to "0", don't
     * allow any exceptions */

    /* return codes from purge() aren't actually used by main() yet. (Maybe they
     * never will be and the function will become of type (void)) */
    return 0;
  }

  short status = 0;

  bool cmd_dry_run = 0;
  char *rmwtrash_env = getenv("RMWTRASH");

  if (rmwtrash_env != NULL)
    cmd_dry_run = strcmp (rmwtrash_env, "dry-run") ? 0 : 1;

  if (cli_user_options->want_empty_trash) {
    puts(_("The contents of all waste folders will be deleted -"));
    if (!user_verify())
    {
      puts(_("Action cancelled."));
      return 0;
    }
  }

  /* if dry-run was enabled, assume verbosity as well */
  if (cmd_dry_run)
    verbose = 1;

  extern const int purge_after;
  printf ("\n");
  if (cli_user_options->want_empty_trash)
    printf (_("Purging all files in waste folders ...\n"));
  else
    printf (_("Purging files based on number of days in the waste folders (%u) ...\n"),
            purge_after);

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;

  while (waste_curr != NULL)
  {
    struct dirent *st_trashinfo_dir_entry;
    DIR *trashinfo_dir = opendir (waste_curr->info);
    if (trashinfo_dir == NULL)
      msg_err_open_dir (waste_curr->info, __func__, __LINE__);

    /*
     *  Read each file in <WASTE>/info
     */
    while ((st_trashinfo_dir_entry = readdir (trashinfo_dir)) != NULL)
    {
      if (!strcmp (st_trashinfo_dir_entry->d_name, ".") || !strcmp (st_trashinfo_dir_entry->d_name, ".."))
        continue;

      int req_len = multi_strlen (waste_curr->info, st_trashinfo_dir_entry->d_name, NULL) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      char trashinfo_entry_realpath[req_len];
      snprintf (trashinfo_entry_realpath, req_len, "%s%s", waste_curr->info, st_trashinfo_dir_entry->d_name);

      /* FIXME: does this condition need modifying? */
      time_t then;
      if (!cli_user_options->want_empty_trash && !(then = get_then_time(st_trashinfo_dir_entry, trashinfo_entry_realpath)))
          continue;

      if (then + (SECONDS_IN_A_DAY * purge_after) <= time_t_now || cli_user_options->want_empty_trash)
      {
        char corresponding_file_to_purge[MP];
        strcpy (corresponding_file_to_purge, waste_curr->files);

        char temp[MP];
        strcpy (temp, st_trashinfo_dir_entry->d_name);
        truncate_str (temp, strlen (DOT_TRASHINFO)); /* acquire the (basename - trashinfo extension) */

        strcat (corresponding_file_to_purge, temp); /* path to file in <WASTE>/files */
        if (lstat (corresponding_file_to_purge, &st))
          msg_err_lstat (__func__, __LINE__);

        int orig_dev = st.st_dev;
        int orig_inode = st.st_ino;

        if (S_ISDIR (st.st_mode))
        {
          if (!cmd_dry_run)
            status = rmdir_recursive (corresponding_file_to_purge, 1, cli_user_options);
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
            printf ("%s\n", corresponding_file_to_purge);
            printf (_("(check owner/write permissions)\n"));
            dirs_containing_files_ctr++;
            break;

          case MAX_DEPTH_REACHED:
            print_msg_warn ();
            /* TRANSLATORS:  "depth" refers to the recursion depth in a
             * directory   */
            printf (_("Maximum depth of %u reached, skipping\n"),
                    RMDIR_MAX_DEPTH);
            printf ("%s\n", corresponding_file_to_purge);
            max_depth_reached_ctr++;
            break;

          case 0:
            if (!cmd_dry_run)
              status = rmdir (corresponding_file_to_purge);
            else
              status = 0;

            if (status == 0)
            {
              deleted_dirs_ctr++;
              bytes_freed += st.st_size;
            }
            else
              msg_err_remove (corresponding_file_to_purge, __func__);
            break;

          default:
              msg_err_remove (corresponding_file_to_purge, __func__);
            break;
          }

        }
        else
        {
          if (!cmd_dry_run)
          {
            if (!is_modified (corresponding_file_to_purge, orig_dev, orig_inode))
              status = remove (corresponding_file_to_purge);
            else status = -1;
          }
          else
            status = 0;
          if (status == 0)
          {
            deleted_files_ctr++;
            bytes_freed += st.st_size;
          }
          else
            msg_err_remove (corresponding_file_to_purge, __func__);
        }

        if (status == 0)
        {
          if (!cmd_dry_run)
            status = remove (trashinfo_entry_realpath);
          else
            status = 0;

          if (!status)
          {
            purge_ctr++;
            if (verbose)
              printf ("-%s\n", corresponding_file_to_purge);
          }
          else
            msg_err_remove (corresponding_file_to_purge, __func__);
        }
      }
    }

    if (closedir (trashinfo_dir))
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
  /* TRANSLATORS: context: "Number of bytes freed" */
  printf (ngettext ("%s freed", "%s freed", bytes_freed),
          human_readable_size (bytes_freed));
  printf ("\n");

  return 0;

}

#ifndef TEST_LIB

short
orphan_maint (st_waste * waste_curr, const char *formatted_str_time_now)
{
  rmw_target st_file_properties;

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  st_file_properties.is_duplicate = 0;

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

      st_file_properties.base_name = basename (entry->d_name);

      int req_len = multi_strlen (waste_curr->info, st_file_properties.base_name,
               DOT_TRASHINFO, NULL) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      snprintf (path_to_trashinfo, req_len, "%s%s%s", waste_curr->info, st_file_properties.base_name,
               DOT_TRASHINFO);

      if (exists (path_to_trashinfo))
        continue;

      /* destination if restored */
      req_len = multi_strlen(waste_curr->parent, "/orphans/", st_file_properties.base_name, NULL) + 1;
      bufchk_len (req_len, MP, __func__, __LINE__);
      snprintf (st_file_properties.real_path, req_len, "%s%s%s", waste_curr->parent, "/orphans/",
               st_file_properties.base_name);

      if (!create_trashinfo (&st_file_properties, waste_curr, formatted_str_time_now))
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

  printf ("%d %s found\n", orphan_ctr, orphan_ctr == 1 ? "orphan" : "orphans");

  return 0;
}
#endif /* TEST_LIB */
