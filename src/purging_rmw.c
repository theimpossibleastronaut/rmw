/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2022  Andy Alt (arch_stanton5995@protonmail.com)
Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fcntl.h>

#include "globals.h"
#include "parse_cli_options.h"
#include "purging_rmw.h"
#include "messages_rmw.h"
#include "strings_rmw.h"
#include "utils_rmw.h"
#include "trashinfo_rmw.h"

#ifndef TEST_LIB
#define RMDIR_MAX_DEPTH 128
#else
#define RMDIR_MAX_DEPTH 32
#endif


/*!
 * remove dirs recursively, primarily used by @ref purge()
 * @param[in] path the directory to be removed
 * @param[out] level keeps track of the number of times recursion has happened
 * @return error number
 */
int
rmdir_recursive(const char *dirname, short unsigned level, const int force,
                st_counters * ctr)
{
  if (level > RMDIR_MAX_DEPTH)
    return RMDIR_MAX_DEPTH;

  int remove_result = 0;

  struct _dirname
  {
    char path[LEN_MAX_PATH];
    DIR *ptr;
    struct dirent *st_entry_ptr;
  } st_dirname_properties;

  st_dirname_properties.ptr = opendir(dirname);
  if (st_dirname_properties.ptr == NULL)
    msg_err_open_dir(dirname, __func__, __LINE__);

  while ((st_dirname_properties.st_entry_ptr =
          readdir(st_dirname_properties.ptr)) != NULL)
  {
    if (isdotdir(st_dirname_properties.st_entry_ptr->d_name))
      continue;

    char *path_tmp =
      join_paths(dirname, st_dirname_properties.st_entry_ptr->d_name, NULL);
    strcpy(st_dirname_properties.path, path_tmp);
    free(path_tmp);
    path_tmp = NULL;

    struct stat st;
    if (lstat(st_dirname_properties.path, &st))
      msg_err_lstat(st_dirname_properties.path, __func__, __LINE__);

    if (force >= 2 && ~st.st_mode & S_IWUSR)
    {
      // using fchmod instead of chmod to hopefully prevent codeql
      // from complaining about TOCTOU warnings
      // https://github.com/theimpossibleastronaut/rmw/security/code-scanning/4?query=ref%3Arefs%2Fheads%2Fmaster
      FILE *fp = fopen(st_dirname_properties.path, "r");
      if (fp == NULL)
      {
        open_err(st_dirname_properties.path, __func__);
        return errno;
      }

      // fchmod requires a file descriptor.
      int fd = fileno(fp);
      if (fd == -1)
      {
        strerror(errno);
        return errno;
      }

      if (fchmod(fd, 00700) == 0)
      {
        /* Now that the mode has changed, lstat must be run again */
        if (lstat(st_dirname_properties.path, &st))
          msg_err_lstat(st_dirname_properties.path, __func__, __LINE__);
      }
      else
      {
        print_msg_error();
        fprintf(stderr, _("while changing permissions of %s\n"), dirname);
        perror("fchmod: ");
        putchar('\n');
        /* if permissions aren't changed, the directory is still
         * not writable. This error shouldn't really happen. I don't
         * know what to do next if that happens. Right now, the program
         * will continue as normal, with the warning message about
         * permissions
         */
      }
      int r = close_file(fp, st_dirname_properties.path, __func__);
      if (r)
        return r;
    }

    if (st.st_mode & S_IWUSR)
    {
      if (!S_ISDIR(st.st_mode))
      {
        if (remove(st_dirname_properties.path) == 0)
        {
          ctr->deleted_files++;
          ctr->bytes_freed += st.st_size;
        }
        else
        {
          perror("rmdir_recursive -> remove");
        }
      }
      else
      {
        remove_result =
          rmdir_recursive(st_dirname_properties.path, ++level, force, ctr);
        level--;

        switch (remove_result)
        {

        case EACCES:
          if (closedir(st_dirname_properties.ptr))
            msg_err_close_dir(dirname, __func__, __LINE__);
          return EACCES;
          break;
        case RMDIR_MAX_DEPTH:
          if (closedir(st_dirname_properties.ptr))
            msg_err_close_dir(dirname, __func__, __LINE__);
          return RMDIR_MAX_DEPTH;
          break;
        }
      }
    }
    else
    {
      printf("\nPermission denied while deleting\n");
      printf("%s\n", st_dirname_properties.path);

      if (closedir(st_dirname_properties.ptr))
        msg_err_close_dir(dirname, __func__, __LINE__);

      return EACCES;
    }
  }

  if (closedir(st_dirname_properties.ptr))
    msg_err_close_dir(dirname, __func__, __LINE__);

  if (level > 1)
  {
    if ((remove_result = rmdir(dirname)) == 0)
      ctr->deleted_dirs++;
    else
      perror("rmdir_recursive -> rmdir");
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
is_time_to_purge(st_time * st_time_var, const char *file)
{
  const int BUF_TIME = 80;

  FILE *fp = fopen(file, "r");
  bool init = (fp);
  if (fp)
  {
    char time_prev[BUF_TIME];

    if (fgets(time_prev, sizeof time_prev, fp) == NULL)
    {
      print_msg_error();
      printf("while getting line from %s\n", file);
      perror(__func__);
      close_file(fp, file, __func__);
      exit(EXIT_FAILURE);
    }

    trim_whitespace(time_prev);
    close_file(fp, file, __func__);

    if ((st_time_var->now - atoi(time_prev)) < SECONDS_IN_A_DAY)
      return false;
  }

  fp = fopen(file, "w");
  if (fp)
  {
    fprintf(fp, "%ld\n", st_time_var->now);
    close_file(fp, file, __func__);

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
  open_err(file, __func__);
  msg_return_code(errno);
  exit(errno);
}


static void
show_purge_stats(st_counters * ctr)
{
  printf(ngettext("%d file purged", "%d files purged", ctr->purge),
         ctr->purge);
  putchar('\n');
  printf(ngettext
         ("(%d file deleted)", "(%d files deleted)", ctr->deleted_files),
         ctr->deleted_files);
  putchar('\n');
  printf(ngettext
         ("(%d directory deleted)", "(%d directories deleted)",
          ctr->deleted_dirs), ctr->deleted_dirs);
  putchar('\n');
  /* TRANSLATORS: context: "Number of bytes freed" */
  char hr_size[LEN_MAX_HUMAN_READABLE_SIZE];
  make_size_human_readable(ctr->bytes_freed, hr_size);
  printf("%s freed", hr_size);
  putchar('\n');

  return;
}

/*!
 * Purges files older than x number of days, unless expire_age is set to
 * 0 in the config file.
 */
int
purge(st_config * st_config_data,
      const rmw_options * cli_user_options,
      st_time * st_time_var, int *orphan_ctr)
{
  if (!st_config_data->expire_age)
  {
    /* TRANSLATORS:  "purging" refers to permanently deleting a file or a
     * directory  */
    printf(_("purging is disabled ('%s' is set to '0')\n\n"), expire_age_str);
    return 0;
  }

  int status = 0;

  if (cli_user_options->want_empty_trash)
  {
    puts(_("The contents of all waste folders will be deleted -"));
    if (!user_verify())
    {
      puts(_("Action cancelled."));
      return 0;
    }
  }

  putchar('\n');
  if (cli_user_options->want_empty_trash)
    printf(_("Purging all files in waste folders ...\n"));
  else
    printf(_
           ("Purging files based on number of days in the waste folders (%u) ...\n"),
           st_config_data->expire_age);

  st_counters ctr = { 0, 0, 0, 0, 0, 0 };

  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
  while (waste_curr != NULL)
  {
    struct dirent *st_trashinfo_dir_entry;
    DIR *trashinfo_dir = opendir(waste_curr->info);
    if (trashinfo_dir == NULL)
      msg_err_open_dir(waste_curr->info, __func__, __LINE__);

    if (verbose)
    {
      putchar('\n');
      printf("  [%s]\n", waste_curr->files);
      printf("  ");
      char *p = waste_curr->files;
      while (*(p++) != '\0')
        printf("-");
      puts("--");
    }

    /*
     *  Read each file in <WASTE>/info
     */
    while ((st_trashinfo_dir_entry = readdir(trashinfo_dir)) != NULL)
    {
      if (isdotdir(st_trashinfo_dir_entry->d_name))
        continue;

      char *tmp_str =
        join_paths(waste_curr->info, st_trashinfo_dir_entry->d_name, NULL);
      char trashinfo_entry_realpath[strlen(tmp_str) + 1];
      strcpy(trashinfo_entry_realpath, tmp_str);
      free(tmp_str);

      char *raw_deletion_date =
        parse_trashinfo_file(trashinfo_entry_realpath, deletion_date_key);
      time_t then = 0;
      if (raw_deletion_date != NULL)
      {
        then = get_then_time(raw_deletion_date);
        free(raw_deletion_date);
      }
      else
      {
        print_msg_error();
        fprintf(stderr, "while getting deletion date from %s.\n",
                trashinfo_entry_realpath);
      }

      if (!cli_user_options->want_empty_trash && !then)
        continue;

      double days_remaining =
        ((double) then + (SECONDS_IN_A_DAY * st_config_data->expire_age) -
         st_time_var->now) / SECONDS_IN_A_DAY;

      bool do_file_purge = days_remaining <= 0
        || cli_user_options->want_empty_trash;
      if (do_file_purge || verbose >= 2)
      {
        char temp[strlen(st_trashinfo_dir_entry->d_name) + 1];
        strcpy(temp, st_trashinfo_dir_entry->d_name);
        truncate_str(temp, len_trashinfo_ext);  /* acquire the (basename - trashinfo extension) */

        char *_tmp_str = join_paths(waste_curr->files, temp, NULL);
        char purge_target[strlen(_tmp_str) + 1];
        strcpy(purge_target, _tmp_str);
        free(_tmp_str);

        char pt_tmp[sizeof purge_target];
        strcpy(pt_tmp, purge_target);
        char *pt_basename = basename(pt_tmp);

        if (do_file_purge)
        {
          // If the corresponding file wasn't found, either display an error and exit, or remove the
          // (probably) orphaned trashinfo file.
          struct stat st;
          if (lstat(purge_target, &st))
          {
            if (cli_user_options->want_orphan_chk
                && cli_user_options->force >= 2)
            {
              int res = 0;
              if (cli_user_options->want_dry_run == false)
              {
                res = remove(trashinfo_entry_realpath);
                if (res != 0)
                  msg_err_remove(trashinfo_entry_realpath, __func__);
              }
              if (res == 0)
                printf("removed '%s'\n", trashinfo_entry_realpath);
              (*orphan_ctr)++;
              continue;
            }
            else
            {
              printf("While processing %s:\n", trashinfo_entry_realpath);
              puts("You can remove the trashinfo file with '-offg'");
              // Will exit after error
              msg_err_lstat(purge_target, __func__, __LINE__);
            }
          }

          if (S_ISDIR(st.st_mode))
          {
            if (cli_user_options->want_dry_run == false)
              status =
                rmdir_recursive(purge_target, 1, cli_user_options->force,
                                &ctr);
            else
            {
              /* Not much choice but to
               * assume there would not be an error if the attempt were actually made */
              status = 0;
            }

            switch (status)
            {
            case EACCES:
              print_msg_warn();
              printf(_("Directory not purged - still contains files\n"));
              printf("%s\n", purge_target);
              printf(_("(check owner/write permissions)\n"));
              ctr.dirs_containing_files++;
              break;

            case RMDIR_MAX_DEPTH:
              print_msg_warn();
              /* TRANSLATORS:  "depth" refers to the recursion depth in a
               * directory   */
              printf(_("Maximum depth of %u reached, skipping\n"),
                     RMDIR_MAX_DEPTH);
              printf("%s\n", purge_target);
              ctr.max_depth_reached++;
              break;

            case 0:
              if (cli_user_options->want_dry_run == false)
                status = rmdir(purge_target);
              else
                status = 0;

              if (status == 0)
              {
                ctr.deleted_dirs++;
                ctr.bytes_freed += st.st_size;
              }
              else
                msg_err_remove(purge_target, __func__);
              break;

            default:
              msg_err_remove(purge_target, __func__);
              break;
            }

          }
          else
          {
            if (cli_user_options->want_dry_run == false)
              status = remove(purge_target);
            else
              status = 0;
            if (status == 0)
            {
              ctr.deleted_files++;
              ctr.bytes_freed += st.st_size;
            }
            else
              msg_err_remove(purge_target, __func__);
          }

          if (status == 0)
          {
            if (cli_user_options->want_dry_run == false)
              status = remove(trashinfo_entry_realpath);
            else
              status = 0;

            if (!status)
            {
              ctr.purge++;
              if (verbose)
                printf("-%s\n", pt_basename);
            }
            else
              msg_err_remove(purge_target, __func__);
          }
        }
        else if (verbose >= 2)
        {
          printf(_("'%s' will be purged in %.2lf days\n"),
                 pt_basename, days_remaining);
        }
      }
    }

    if (closedir(trashinfo_dir))
      msg_err_close_dir(waste_curr->info, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  if (ctr.max_depth_reached)
    printf(_("%d directories skipped (RMDIR_MAX_DEPTH reached)\n"),
           ctr.max_depth_reached);

  if (ctr.dirs_containing_files)
    printf(_("%d directories skipped (contains read-only files)\n"),
           ctr.dirs_containing_files);

  show_purge_stats(&ctr);

  return 0;

}

short
orphan_maint(st_waste * waste_head, st_time * st_time_var, int *orphan_ctr)
{
  rmw_target st_file_properties;

  /* searching for files that don't have a trashinfo: There will
   * never be a duplicate, but it initializes the struct member, and
   * create_trashinfo() will check for it, which is called later.
   */
  st_file_properties.is_duplicate = 0;

  char path_to_trashinfo[LEN_MAX_PATH];
  st_waste *waste_curr = waste_head;
  while (waste_curr != NULL)
  {
    struct dirent *entry;
    DIR *files;
    files = opendir(waste_curr->files);
    if (files == NULL)
      msg_err_open_dir(waste_curr->files, __func__, __LINE__);

    while ((entry = readdir(files)) != NULL)
    {
      if (isdotdir(entry->d_name))
        continue;

      st_file_properties.base_name = basename(entry->d_name);

      char *tmp_str =
        join_paths(waste_curr->info, st_file_properties.base_name, NULL);
      int r = snprintf(path_to_trashinfo, LEN_MAX_PATH, "%s%s", tmp_str,
                       trashinfo_ext);
      sn_check(r, LEN_MAX_PATH, __func__, __LINE__);

      free(tmp_str);

      if (exists(path_to_trashinfo))
        continue;

      /* destination if restored */
      st_file_properties.real_path =
        join_paths(waste_curr->parent, "orphans",
                   st_file_properties.base_name, NULL);

      if (!create_trashinfo(&st_file_properties, waste_curr, st_time_var))
      {
        /* TRANSLATORS:  "created" refers to a file  */
        printf(_("Created %s\n"), path_to_trashinfo);
        (*orphan_ctr)++;
      }
      else
      {
        print_msg_error();
        printf(_("while creating %s\n"), path_to_trashinfo);
      }
      free(st_file_properties.real_path);

    }
    if (closedir(files))
      msg_err_close_dir(waste_curr->files, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  printf("%d %s found\n", *orphan_ctr,
         *orphan_ctr == 1 ? "orphan" : "orphans");

  return 0;
}

///////////////////////////////////////////////////////////////////////
#ifdef TEST_LIB

#include "test.h"

static void
test_rmdir_recursive(void)
{
  st_counters ctr = { 0, 0, 0, 0, 0, 0 };
  char cur_dir[LEN_MAX_PATH];
  assert(getcwd(cur_dir, LEN_MAX_PATH) != NULL);

  // Just an extra trivial check to make sure that the number used for
  // the test matches with what's defined in purging_rmw.h
  assert(RMDIR_MAX_DEPTH == 32);

  char dir_rmdir_test[LEN_MAX_PATH];
  strcpy(dir_rmdir_test, "rmdir_test");

  FILE *fd1;
  FILE *fd2;

  int i;
  for (i = 0; i < RMDIR_MAX_DEPTH; i++)
  {
    assert(mkdir(dir_rmdir_test, S_IRWXU) == 0);

    if (i == RMDIR_MAX_DEPTH - 1)
    {
      // Make the last directory non-writable
      assert(chmod(dir_rmdir_test, 00500) == 0);
    }

    // These files can't get created after a dir has permissions set to
    // 00500, so making it conditional
    if (i != RMDIR_MAX_DEPTH - 1)
    {
      assert(chdir(dir_rmdir_test) == 0);
      assert((fd1 = fopen("test_file1", "w+")) != NULL);
      assert(fclose(fd1) == 0);
      assert((fd2 = fopen("test file2", "w+")) != NULL);
      assert(chmod("test file2", 00400) == 0);
      assert(fclose(fd2) == 0);
    }
    printf("%d\n", i);
  }

  // Go back to the original cwd
  assert(chdir(cur_dir) == 0);

  int force = 1;
  int level = 1;

  // Because some of the created files don't have write permission, this
  // should fail the first time when force isn't set to 2
  assert(rmdir_recursive(dir_rmdir_test, level, force, &ctr) == EACCES);

  force = 2;
  // Now it should pass
  assert(rmdir_recursive(dir_rmdir_test, level, force, &ctr) == 0);
  assert(rmdir(dir_rmdir_test) == 0);

  // Now try the test again, but creating a number of dirs that exceed MAX_DEPTH
  for (i = 0; i < RMDIR_MAX_DEPTH + 1; i++)
  {
    assert(mkdir(dir_rmdir_test, S_IRWXU) == 0);
    assert(chdir(dir_rmdir_test) == 0);
  }

  // Go back to the original cwd
  assert(chdir(cur_dir) == 0);

  // should return an error because MAX_DEPTH was reached
  assert(rmdir_recursive(dir_rmdir_test, level, force, &ctr) ==
         RMDIR_MAX_DEPTH);
  printf("deleted_dirs: %d\n", ctr.deleted_dirs);
  assert(ctr.deleted_dirs == RMDIR_MAX_DEPTH - 1);

  // Change the 'level' argument so it will go one level further down
  level = 0;
  assert(rmdir_recursive(dir_rmdir_test, level, force, &ctr) == 0);

  level = 1;
  assert(rmdir_recursive(dir_rmdir_test, level, force, &ctr) == 0);

  // remove the top directory, which should now be empty
  assert(rmdir(dir_rmdir_test) == 0);

  return;
}


int
main(void)
{
  test_rmdir_recursive();
  return 0;
}
#endif
