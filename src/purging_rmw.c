/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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
#include "main.h"
#include "parse_cli_options.h"
#include "config_rmw.h"
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
    char path[LEN_MAX_PATH];
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

    bufchk (dirname, LEN_MAX_PATH);
    strncpy (st_dirname_properties.path, dirname, LEN_MAX_PATH - 1);

    int pathLen = strlen (st_dirname_properties.path);
    if (st_dirname_properties.path[pathLen - 1] != '/')
    {
      st_dirname_properties.path[pathLen] = '/';
      pathLen++;
      st_dirname_properties.path[pathLen] = '\0';
    }

    int req_len = multi_strlen (st_dirname_properties.path, st_dirname_properties.st_entry_ptr->d_name, NULL) + 1;
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    strcat (st_dirname_properties.path, st_dirname_properties.st_entry_ptr->d_name);

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
 * Called in main() to determine whether or not purge() was run today, reads
 * and writes to the 'lastpurge` file. If it hasn't been run today, the
 * current day will be written. If 'lastpurge' doesn't exist, it gets
 * created.
 */
bool
is_time_to_purge (st_time *st_time_var, const char* data_dir)
{
  const int BUF_TIME = 80;
  const char purge_time_file[] = "purge-time";
  int req_len = multi_strlen (data_dir, "/", purge_time_file, NULL);
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char file_lastpurge[req_len + 1];
  sprintf (file_lastpurge, "%s/%s", data_dir, purge_time_file);

  FILE *fp = fopen (file_lastpurge, "r");
  bool init = (fp);
  if (fp)
  {
    char time_prev[BUF_TIME];

    if (fgets (time_prev, sizeof time_prev, fp) == NULL)
    {
      print_msg_error ();
      printf ("while getting line from %s\n", file_lastpurge);
      perror (__func__);
      close_file (fp, file_lastpurge, __func__);
      exit (ERR_FGETS);
    }

    trim_white_space (time_prev);
    close_file (fp, file_lastpurge, __func__);

    if ((st_time_var->now - atoi (time_prev)) < SECONDS_IN_A_DAY)
      return false;
  }

  fp = fopen (file_lastpurge, "w");
  if (fp)
  {
    fprintf (fp, "%ld\n", st_time_var->now);
    close_file (fp, file_lastpurge, __func__);

    /*
     * if this is the first time the file got created, it's very likely
     * indeed that purge does not need to run. Only return FALSE if the
     * file didn't previously exist.
     */
    return init;
  }

  /*
   * if we can't even write this file to the config directory, something
   * is not right. Make it fatal.
   */
  open_err (file_lastpurge, __func__);
  msg_return_code (ERR_OPEN);
  exit (ERR_OPEN);
}


/*!
 * Purges files older than x number of days, unless purge_after is set to
 * 0 in the config file.
 */
int
purge (
  st_config *st_config_data,
  const rmw_options * cli_user_options,
  st_time *st_time_var)
{
  if (!st_config_data->purge_after)
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

  if (cli_user_options->want_empty_trash) {
    puts(_("The contents of all waste folders will be deleted -"));
    if (!user_verify())
    {
      puts(_("Action cancelled."));
      return 0;
    }
  }

  printf ("\n");
  if (cli_user_options->want_empty_trash)
    printf (_("Purging all files in waste folders ...\n"));
  else
    printf (_("Purging files based on number of days in the waste folders (%u) ...\n"),
            st_config_data->purge_after);

  struct stat st;

  unsigned int purge_ctr = 0;
  unsigned int dirs_containing_files_ctr = 0;
  unsigned int max_depth_reached_ctr = 0;
  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
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
      bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
      char trashinfo_entry_realpath[req_len];
      sprintf (trashinfo_entry_realpath, "%s%s", waste_curr->info, st_trashinfo_dir_entry->d_name);

      time_t then = get_then_time(trashinfo_entry_realpath);
      if (!cli_user_options->want_empty_trash && !then)
          continue;

      if (then + (SECONDS_IN_A_DAY * st_config_data->purge_after) <= st_time_var->now ||
          cli_user_options->want_empty_trash)
      {
        char corresponding_file_to_purge[LEN_MAX_PATH];
        strcpy (corresponding_file_to_purge, waste_curr->files);

        char temp[LEN_MAX_PATH];
        strcpy (temp, st_trashinfo_dir_entry->d_name);
        truncate_str (temp, strlen (DOT_TRASHINFO)); /* acquire the (basename - trashinfo extension) */

        strcat (corresponding_file_to_purge, temp); /* path to file in <WASTE>/files */
        if (lstat (corresponding_file_to_purge, &st))
          msg_err_lstat (__func__, __LINE__);

        int orig_dev = st.st_dev;
        int orig_inode = st.st_ino;

        if (S_ISDIR (st.st_mode))
        {
          if (!cli_user_options->want_dry_run)
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
            if (!cli_user_options->want_dry_run)
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
          if (!cli_user_options->want_dry_run)
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
          if (!cli_user_options->want_dry_run)
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
orphan_maint (st_waste * waste_head, st_time *st_time_var)
{
  rmw_target st_file_properties;

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  st_file_properties.is_duplicate = 0;

  char path_to_trashinfo[LEN_MAX_PATH];
  int orphan_ctr = 0;
  st_waste *waste_curr = waste_head;
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
      bufchk_len (req_len, sizeof path_to_trashinfo, __func__, __LINE__);
      snprintf (path_to_trashinfo, req_len, "%s%s%s", waste_curr->info, st_file_properties.base_name,
               DOT_TRASHINFO);

      if (exists (path_to_trashinfo))
        continue;

      /* destination if restored */
      req_len = multi_strlen(waste_curr->parent, "/orphans/", st_file_properties.base_name, NULL) + 1;
      bufchk_len (req_len, sizeof st_file_properties.real_path, __func__, __LINE__);
      sprintf (st_file_properties.real_path, "%s%s%s", waste_curr->parent, "/orphans/",
               st_file_properties.base_name);

      if (!create_trashinfo (&st_file_properties, waste_curr, st_time_var))
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
