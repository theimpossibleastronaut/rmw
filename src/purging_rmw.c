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

typedef struct
{
  path_string target;
  char *target_basename;
  bool want;
  struct dirent *st_ti_ent;
  path_string ti_ent_realpath;

} _st_purge;

enum
{
  CONTINUE
};

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


/*!
 * Get the time a file was rmw'ed by reading the corresponding trashinfo
 * file. Called from purge()
 */
static time_t
get_then_time(const char *tinfo_file)
{
  char *raw_deletion_date =
    parse_trashinfo_file(tinfo_file, deletion_date_key);
  if (raw_deletion_date != NULL)
  {
    struct tm tm_then;
    memset(&tm_then, 0, sizeof(struct tm));
    strptime(raw_deletion_date, "%Y-%m-%dT%H:%M:%S", &tm_then);
    free(raw_deletion_date);
    return mktime(&tm_then);
  }
  print_msg_error();
  fprintf(stderr, "while getting deletion date from %s.\n", tinfo_file);
  return 0;
}


static void
print_header(char *files_dir)
{
  putchar('\n');
  printf("  [%s]\n", files_dir);
  printf("  ");
  char *p = files_dir;
  while (*(p++) != '\0')
    printf("-");
  puts("--");
}

static int
do_file_purge(_st_purge * st_purge, const rmw_options * cli_user_options,
              int *orphan_ctr, st_rm * rm, int *ctr)
{
  int status = 0;
  // If the corresponding file wasn't found, either display an error and exit, or remove the
  // (probably) orphaned trashinfo file.
  struct stat st;
  if (lstat(st_purge->target, &st))
  {
    if (cli_user_options->want_orphan_chk && cli_user_options->force >= 2)
    {
      int res = 0;
      if (cli_user_options->want_dry_run == false)
      {
        res = remove(st_purge->ti_ent_realpath);
        if (res != 0)
          msg_err_remove(st_purge->ti_ent_realpath, __func__);
      }
      if (res == 0)
        printf("removed '%s'\n", st_purge->ti_ent_realpath);
      (*orphan_ctr)++;
      return CONTINUE;
    }
    else
    {
      printf("While processing %s:\n", st_purge->ti_ent_realpath);
      puts("You can remove the trashinfo file with '-offg'");
      // Will exit after error
      msg_err_lstat(st_purge->target, __func__, __LINE__);
    }
  }

  if (!S_ISDIR(st.st_mode))
  {
    if (cli_user_options->want_dry_run == false)
      status = remove(st_purge->target);
    else
      status = 0;
  }
  else
  {
    char rm_cmd[LEN_MAX_RM_CMD];
    sn_check(snprintf(rm_cmd,
                      sizeof rm_cmd,
                      "%s -rf %s %s %s",
                      rm->full_path,
                      rm->v, rm->onefs, st_purge->target), sizeof rm_cmd);

    if (cli_user_options->want_dry_run == false)
      status = system(rm_cmd);
    else
    {
      printf("removing '%s\n", rm_cmd);
      /* Not much choice but to
       * assume there would not be an error if the attempt were actually made */
      status = 0;
    }
  }

  if (status == 0)
  {
    if (cli_user_options->want_dry_run == false)
      status = remove(st_purge->ti_ent_realpath);
    else
      status = 0;

    if (!status)
    {
      (*ctr)++;
      if (verbose)
        printf("-%s\n", st_purge->target_basename);
    }
    else
      msg_err_remove(st_purge->ti_ent_realpath, __func__);
  }
  else
  {
    msg_err_remove(st_purge->target, __func__);
  }
  return 0;
}


static char *
get_pt_basename(const char *target)
{
  static char *pt_basename;
  static path_string pt_tmp;
  strcpy(pt_tmp, target);
  pt_basename = basename(pt_tmp);
  return pt_basename;
}


void
init_rm(st_rm * rm)
{
  sn_check(snprintf(rm->full_path, LEN_MAX_PATH, RM_FULL_PATH), LEN_MAX_PATH);
  strcpy(rm->onefs, ONEFS_STR);
  strcpy(rm->v, "-v");

#ifndef RM_HAS_ONE_FILE_SYSTEM_ARG
  *rm->onefs = '\0';
#endif
  if (!verbose)
    *rm->v = '\0';
  char *appdir = getenv("APPDIR");
  if (appdir != NULL)
    sn_check(snprintf
             (rm->full_path, LEN_MAX_PATH, "%s/usr/bin/rm", appdir),
             LEN_MAX_PATH);
}

static void
get_purge_target(_st_purge * st_purge, const char *files_dir)
{
  path_string temp;
  sn_check(snprintf(temp, sizeof temp, "%s", st_purge->st_ti_ent->d_name),
           sizeof temp);
  truncate_str(temp, len_trashinfo_ext);        /* acquire the (basename - trashinfo extension) */
  *st_purge->target = '\0';
  join_paths2(st_purge->target, sizeof_memb(_st_purge, target), files_dir,
              temp);
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

  st_rm rm;
  init_rm(&rm);

  _st_purge st_purge = {
    "",
    NULL,
    0,
    NULL,
    ""
  };

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

  int ctr = 0;

  st_waste *waste_curr = st_config_data->st_waste_folder_props_head;
  while (waste_curr != NULL)
  {
    DIR *trashinfo_dir = opendir(waste_curr->info);
    if (trashinfo_dir == NULL)
      msg_err_open_dir(waste_curr->info, __func__, __LINE__);

    if (verbose)
      print_header(waste_curr->files);

    /*
     *  Read each file in <WASTE>/info
     */
    while ((st_purge.st_ti_ent = readdir(trashinfo_dir)) != NULL)
    {
      if (isdotdir(st_purge.st_ti_ent->d_name))
        continue;

      *st_purge.ti_ent_realpath = '\0';
      join_paths2(st_purge.ti_ent_realpath, sizeof st_purge.ti_ent_realpath,
                  waste_curr->info, st_purge.st_ti_ent->d_name);

      time_t then = get_then_time(st_purge.ti_ent_realpath);
      if (!cli_user_options->want_empty_trash && !then)
        continue;

      double days_remaining =
        ((double) then + (SECONDS_IN_A_DAY * st_config_data->expire_age) -
         st_time_var->now) / SECONDS_IN_A_DAY;

      st_purge.want = days_remaining <= 0
        || cli_user_options->want_empty_trash;
      if (st_purge.want || verbose >= 2)
      {
        get_purge_target(&st_purge, waste_curr->files);
        st_purge.target_basename = get_pt_basename(st_purge.target);

        if (st_purge.want)
        {
          if (do_file_purge(&st_purge, cli_user_options,
                            orphan_ctr, &rm, &ctr) == CONTINUE)
            continue;
        }
        else if (verbose >= 2)
        {
          printf(_("'%s' will be purged in %.2lf days\n"),
                 st_purge.target_basename, days_remaining);
        }
      }
    }

    if (closedir(trashinfo_dir))
      msg_err_close_dir(waste_curr->info, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  printf(ngettext("%d item purged", "%d items purged", ctr), ctr);
  putchar('\n');

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

  path_string path_to_trashinfo;
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

      path_string tmp_str;
      *tmp_str = '\0';
      join_paths2(tmp_str, sizeof_memb(st_waste, info), waste_curr->info,
                  st_file_properties.base_name);
      int r = snprintf(path_to_trashinfo, LEN_MAX_PATH, "%s%s", tmp_str,
                       trashinfo_ext);
      sn_check(r, LEN_MAX_PATH);

      if (exists(path_to_trashinfo))
        continue;

      /* destination if restored */
      *st_file_properties.real_path = '\0';
      join_paths2(st_file_properties.real_path,
                  sizeof st_file_properties.real_path * 2, waste_curr->parent,
                  "orphans", st_file_properties.base_name);

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
    }
    if (closedir(files))
      msg_err_close_dir(waste_curr->files, __func__, __LINE__);

    waste_curr = waste_curr->next_node;
  }

  printf("%d %s found\n", *orphan_ctr,
         *orphan_ctr == 1 ? "orphan" : "orphans");

  return 0;
}
