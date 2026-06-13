/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2025  Andy Alt (arch_stanton5995@proton.me)
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

#include "globals.h"
#include "main.h"
#include "utils.h"
#include "restore.h"
#include "config_rmw.h"
#include "purging.h"
#include "strings_rmw.h"
#include "messages.h"
#include "ficlone.h"
#include "trashinfo.h"
#include "topdir_trash.h"


/*!
 * Assigns a time string to *tm_str based on the format requested
 */
static void
set_time_string(char *tm_str, const size_t len, const char *format,
                time_t time_t_now)
{
  struct tm result;
  if (localtime_r(&time_t_now, &result) == NULL)
    diag_fatal(EXIT_FAILURE,
               "localtime_r() failed for time_t value beyond 32-bit limit.\n");
  strftime(tm_str, len, format, &result);
  trim_whitespace(tm_str);

  return;
}

/*!
 * Returns a formatted string based on whether or not
 * a fake year is requested at runtime. If a fake-year is not requested,
 * the returned string will be based on the local-time of the user's system.
 */
static void
set_which_deletion_date(char *format)
{
  char *fake_year = getenv("RMW_FAKE_YEAR");
  bool valid_value = false;
  if (fake_year != NULL)
  {
    valid_value = strcasecmp(fake_year, "true") == 0;
    puts("RMW_FAKE_YEAR:true");
  }
  sn_check(snprintf
           (format, LEN_MAX_DELETION_DATE, "%s",
            valid_value ? "1999-%m-%dT%T" : "%FT%T"), LEN_MAX_DELETION_DATE);

  return;
}


void
init_time_vars(st_time *x)
{
  x->now = time(NULL);
  // x->now = 0x80000000;

  set_which_deletion_date(x->t_fmt);

  set_time_string(x->deletion_date, LEN_MAX_DELETION_DATE, x->t_fmt, x->now);

  set_time_string(x->suffix_added_dup_exists,
                  LEN_MAX_TIME_STR_SUFFIX, "_%H%M%S-%y%m%d", x->now);

  return;
}


static int
process_mrl(st_waste *waste_head,
            st_time *st_time_var,
            const char *mrl_file, rmw_options *cli_user_options)
{
  enum
  {
    MRL_IS_EMPTY = 10,
  };
  char *contents = NULL;
  // fprintf(stderr, "mrl_file: %s\n", mrl_file);
  bool backwards_compat_empty_header = false;
  FILE *fp = fopen(mrl_file, "r");

  if (fp != NULL)
  {
    fseek(fp, 0, SEEK_END);     // move to the end of the file so we can use ftell()
    const int f_size = ftell(fp);       // Get the size of the file
    rewind(fp);

    contents = calloc(1, f_size + 1);
    if (!contents)
      fatal_malloc();

    int n_items = fread(contents, 1, f_size + 1, fp);
    if (n_items != f_size)
      diag(DIAG_WARN, "ftell() returned %d, but fread() returned %d", f_size,
           n_items);
    if (feof(fp) == 0)
    {
      diag(DIAG_ERR, "while reading %s\n", mrl_file);
      clearerr(fp);
    }
    close_file(&fp, mrl_file, __func__);
    contents[f_size] = '\0';
    if (!strcmp(contents, "[Empty]\n"))
      backwards_compat_empty_header = true;
  }
  else
  {
    if (errno == ENOENT)
    {
      // fprintf(stderr, "%d %s\n", errno, strerror(ENOENT));
      puts(_("There are no items in the list - please check back later.\n"));
      return MRL_IS_EMPTY;
    }
    else
    {
      fprintf(stderr, "open mrl failed: %s\n", strerror(errno));
      return -1;
    }
  }

  int res = 0;

  if (cli_user_options->most_recent_list)
  {
    printf("%s", contents);
    if (cli_user_options->want_undo)
      puts(_
           ("Skipping --undo-last because --most-recent-list was requested"));
  }
  else
  {
    if (backwards_compat_empty_header == false)
      res =
        undo_last_rmw(st_time_var, cli_user_options, contents, waste_head);

    if (res == 0)
    {
      if (cli_user_options->want_dry_run == false)
        if (unlink(mrl_file) != 0)
          fprintf(stderr, "unlink: %s \n%s in %s\n", mrl_file,
                  strerror(errno), __func__);
    }
  }

  free(contents);
  return res;
}


/*!
 * When a file has been successfully rmw'ed, add the filename to
 * a linked list.
 */
static st_removed *
add_removal(st_removed *removals, const char *file)
{
  if (removals == NULL)
  {
    st_removed *temp_node = (st_removed *) malloc(sizeof(st_removed));
    if (!temp_node)
      fatal_malloc();
    removals = temp_node;
  }
  else
  {
    if (!(removals->next_node = (st_removed *) malloc(sizeof(st_removed))))
      fatal_malloc();
    removals = removals->next_node;
  }
  removals->next_node = NULL;
  bufchk_len(strlen(file) + 1, sizeof removals->file, __func__, __LINE__);
  strcpy(removals->file, file);
  return removals;
}


/*!
 * Create a new undo_file (lastrmw)
 */
static void
create_undo_file(st_removed *removals_head, const char *mrl_file)
{
  FILE *fp = fopen(mrl_file, "w");
  if (fp)
  {
    st_removed *st_removals_list = removals_head;
    while (st_removals_list != NULL)
    {
      fprintf(fp, "%s\n", st_removals_list->file);
      st_removals_list = st_removals_list->next_node;
    }
    close_file(&fp, mrl_file, __func__);
  }
  else
    open_err(mrl_file, __func__);

  return;
}


/*!
 * recursively remove all nodes of an object of type @ref st_removed
 */
static void
dispose_removed(st_removed *node)
{
  if (node != NULL)
  {
    dispose_removed(node->next_node);
    free(node);
  }

  return;
}


static void
list_waste_folders(st_waste *waste_head)
{
  // Directories that are not on attached medium are not included in
  // the waste linked list, so we can just assign true here.
  const bool is_attached = true;

  st_waste *waste_curr = waste_head;
  while (waste_curr != NULL)
  {
    show_folder_line(waste_curr->parent, waste_curr->removable, is_attached,
                     waste_curr->no_deposit);
    waste_curr = waste_curr->next_node;
  }

  return;
}

static int
remove_to_waste(const int argc,
                char *const argv[],
                st_waste *waste_head,
                st_time *st_time_var,
                const st_loc *st_location,
                const rmw_options *cli_user_options)
{
  rmw_target st_target;

  st_removed *confirmed_removals_list = NULL;
  st_removed *confirmed_removals_list_head = NULL;

  int n_err = 0;
  int removed_files_ctr = 0;
  int file_arg;
  /* The topdir fallback retries via file_arg--; cap it to one attempt per
     file so a fallback trash that still can't receive the file (e.g. an
     unresolvable mount) can't loop forever. */
  int fb_attempted_arg = -1;
  for (file_arg = optind; file_arg < argc; file_arg++)
  {
    if (*argv[file_arg] == '\0')
    {
      puts("skipping empty string");
      continue;
    }

    char tmp[PATH_MAX];
    sn_check(snprintf(tmp, sizeof(tmp), "%s", argv[file_arg]), sizeof(tmp));

    trim_char('/', tmp);

    /* an argument that's all slashes (e.g. "/", "//") trims to nothing:
       that's the root directory */
    if (*tmp == '\0')
    {
      puts(_("\n\
Your single slash has been ignored. You walk to the market\n\
in the town square and purchase a Spear of Destiny. You walk to\n\
the edge of the forest and find your enemy. You attack, causing\n\
damage of 5000 hp. You feel satisfied.\n"));
      continue;
    }

    char arg[PATH_MAX];
    sn_check(snprintf(arg, sizeof(arg), "%s", tmp), sizeof(arg));

    gchar *_bn = g_path_get_basename(tmp);
    sn_check(snprintf(tmp, sizeof(tmp), "%s", _bn), sizeof(tmp));
    g_free(_bn);
    st_target.base_name = tmp;
    if (isdotdir(st_target.base_name))
    {
      printf("refusing to ReMove '.' or '..' directory: skipping '%s'\n",
             argv[file_arg]);
      continue;
    }

    int p_state = check_pathname_state(arg);
    if (p_state != EEXIST)
    {
      if (p_state == ENOENT)
        msg_warn_file_not_found(argv[file_arg]);

      continue;
    }

    struct stat st_file_arg;
    if (!lstat(arg, &st_file_arg))
    {
      st_target.dev_num = st_file_arg.st_dev;
      st_target.real_path = resolve_path(arg, st_target.base_name);
      if (st_target.real_path == NULL)
      {
        n_err++;
        continue;
      }
    }
    else
    {
      diag(DIAG_WARN, "lstat: (argv[file_arg]) %s\n", strerror(errno));
      continue;
    }

    if (strcmp(st_target.real_path, st_location->home_dir) == 0)
    {
      puts(_("Skipping requested ReMoval of your HOME directory"));
      free(st_target.real_path);
      continue;
    }

    if (!cli_user_options->want_top_level_bypass)
    {
      if (count_chars('/', st_target.real_path) == 1)
      {
        printf(_("Skipping requested ReMoval of %s\n"), st_target.real_path);
        free(st_target.real_path);
        continue;
      }
    }

    /* Make sure the file isn't a waste folder or a file within a waste folder */
    bool is_protected = 0;
    st_waste *waste_curr = waste_head;
    while (waste_curr != NULL)
    {
      if (strncmp
          (waste_curr->parent, st_target.real_path,
           strlen(waste_curr->parent)) == 0)
      {
        diag(DIAG_WARN,
             _("%s resides within a waste folder and has been ignored\n"),
             argv[file_arg]);
        is_protected = 1;
        if (getenv("RMW_FAKE_HOME"))
          n_err++;
        break;
      }
      waste_curr = waste_curr->next_node;
    }
    if (is_protected)
    {
      free(st_target.real_path);
      continue;
    }

    /**
     * cycle through wasteDirs to see which one matches
     * device number of file.orig. Once found, the ReMoval
     * happens (provided all the tests are passed.
     */
    bool src_is_ficlone = is_ficlone_fs(arg);
    waste_curr = waste_head;
    while (waste_curr != NULL)
    {
      /* "!WASTE" folders are never a removal destination */
      if (waste_curr->no_deposit)
      {
        waste_curr = waste_curr->next_node;
        continue;
      }
      if (waste_curr->dev_num == st_target.dev_num ||
          (waste_curr->is_ficlone_fs && src_is_ficlone))
      {
        char *tmp_str = join_paths(waste_curr->files, st_target.base_name);
        // *st_target.waste_dest_name = '\0';
        strcpy(st_target.waste_dest_name, tmp_str);
        free(tmp_str);
        tmp_str = NULL;

        /* If a duplicate file exists
         */
        if ((st_target.is_duplicate =
             (check_pathname_state(st_target.waste_dest_name)) == EEXIST))
        {
          // append a time string
          bufchk_len(strlen(st_target.waste_dest_name) +
                     LEN_MAX_TIME_STR_SUFFIX,
                     sizeof st_target.waste_dest_name, __func__, __LINE__);
          strcat(st_target.waste_dest_name,
                 st_time_var->suffix_added_dup_exists);
        }

        int r_result = 0;
        if (cli_user_options->want_dry_run == false)
        {
          const char *src = arg;
          const char *dst = st_target.waste_dest_name;

          if (waste_curr->dev_num != st_target.dev_num)
          {
            r_result = ficlone_move(src, dst);
            if (r_result != 0)
            {
              waste_curr = waste_curr->next_node;
              continue;
            }
          }
          else
          {
            /* same device: simple rename */
            r_result = rename(src, dst);
            if (r_result != 0 && errno == EXDEV)
            {
              /* Same device but a different mount (e.g. a bind mount):
                 rename can't cross it. Try reflink only where it can
                 work; otherwise skip this waste so the $topdir fallback
                 below runs. */
              if (src_is_ficlone)
                r_result = ficlone_move(src, dst);
              if (r_result != 0)
              {
                waste_curr = waste_curr->next_node;
                continue;
              }
            }
          }
        }

        if (r_result == 0)
        {
          verbose_printf(1, "'%s' -> '%s'\n", argv[file_arg],
                         st_target.waste_dest_name);

          removed_files_ctr++;

          if (cli_user_options->want_dry_run == false)
            if (!create_trashinfo(&st_target, waste_curr, st_time_var))
            {
              free(st_target.real_path);
              confirmed_removals_list =
                add_removal(confirmed_removals_list,
                            st_target.waste_dest_name);
              if (confirmed_removals_list_head == NULL)
                confirmed_removals_list_head = confirmed_removals_list;
            }
        }
        else
          msg_err_rename(argv[file_arg], st_target.waste_dest_name);

    /**
     * If we get to this point, it means a WASTE folder was found
     * that matches the file system that file->orig was on.
     * breaking from the for loop
     */
        break;
      }

      /* If the file didn't match with a waste folder on the same filesystem,
       * try the next waste folder */
      waste_curr = waste_curr->next_node;
    }

    if (!waste_curr)
    {
      /* No configured WASTE matched. Fall back to the spec-compliant
       * $topdir trash for this file's mount, creating it on demand.
       * Use real_path: g_unix_mount_for() needs an absolute path. */
      char *fb_mount = NULL;
      char *fb_trash =
        find_topdir_trash(st_target.real_path, get_user_uid_str(),
                          &fb_mount);
      char *fb_files = NULL;
      char *fb_info = NULL;
      bool fb_ok = false;
      if (fb_trash != NULL)
      {
        fb_files = join_paths(fb_trash, "files");
        fb_info = join_paths(fb_trash, "info");
        fb_ok = true;
        if (check_pathname_state(fb_files) == ENOENT
            && rmw_mkdir(fb_files) != 0)
          fb_ok = false;
        if (fb_ok
            && check_pathname_state(fb_info) == ENOENT
            && rmw_mkdir(fb_info) != 0)
          fb_ok = false;
      }

      if (fb_ok && fb_attempted_arg != file_arg)
      {
        struct stat fb_st;
        if (lstat(fb_trash, &fb_st) != 0)
          fb_ok = false;
        else
        {
          st_waste *new_node = malloc(sizeof *new_node);
          if (!new_node)
            fatal_malloc();
          new_node->parent = fb_trash;
          new_node->files = fb_files;
          new_node->info = fb_info;
          new_node->len_files = strlen(fb_files);
          new_node->len_info = strlen(fb_info);
          new_node->media_root = fb_mount;       /* takes ownership */
          fb_mount = NULL;
          new_node->removable = false;
          new_node->no_deposit = false;
          new_node->is_ficlone_fs = is_ficlone_fs(fb_trash);
          new_node->dev_num = fb_st.st_dev;
          new_node->next_node = NULL;

          st_waste *tail = waste_head;
          while (tail->next_node != NULL)
            tail = tail->next_node;
          new_node->prev_node = tail;
          tail->next_node = new_node;

          verbose_printf(1, "fallback trash: %s\n", fb_trash);
          fb_attempted_arg = file_arg;
          free(st_target.real_path);
          file_arg--;
          continue;
        }
      }

      free(fb_trash);
      free(fb_files);
      free(fb_info);
      free(fb_mount);
      printf(_(" :'%s' not ReMoved:\n"), argv[file_arg]);
      printf(_
             ("No WASTE folder defined in '%s' that resides on the same filesystem.\n"),
             st_location->config_file);
      free(st_target.real_path);
    }
  }

  if (confirmed_removals_list_head != NULL)
  {
    create_undo_file(confirmed_removals_list_head, st_location->mrl_file);
    dispose_removed(confirmed_removals_list_head);
  }

  printf(ngettext
         ("%d item was removed to the waste folder",
          "%d items were removed to the waste folder", removed_files_ctr),
         removed_files_ctr);
  putchar('\n');

  return n_err;
}


/* Returns a newly-malloc'd path to $XDG_DATA_HOME/Trash (or
 * $home_dir/.local/share/Trash if XDG_DATA_HOME is unset). */
static char *
home_trash_dir_for(const char *home_dir)
{
  const char *xdg_data = getenv("XDG_DATA_HOME");
  if (xdg_data != NULL && *xdg_data != '\0')
    return join_paths(xdg_data, "Trash");
  char *xdg_default = join_paths(home_dir, ".local/share");
  char *trash = join_paths(xdg_default, "Trash");
  free(xdg_default);
  return trash;
}


/* After parse_config_file(), augment the in-memory waste list with any
 * spec-compliant topdir trash directories that already exist on disk.
 * This lets restore/list/purge/orphan_maint discover trashes that were
 * created by the remove_to_waste() fallback in earlier invocations
 * (i.e. trash dirs not listed in rmwrc). */
static void
discover_existing_topdir_trashes(st_config *st_config_data,
                                 const st_loc *st_location)
{
  char *home_trash_dir = home_trash_dir_for(st_location->home_dir);

  struct stat st;
  dev_t home_dev = 0;
  if (lstat(st_location->home_dir, &st) == 0)
    home_dev = st.st_dev;

  st_mount_trash *mts =
    build_mount_trash_list(st_config_data->uid, home_trash_dir, home_dev);

  for (st_mount_trash *n = mts; n != NULL; n = n->next)
  {
    if (check_pathname_state(n->trash_dir) != EEXIST)
      continue;
    if (check_pathname_state(n->files_dir) != EEXIST)
      continue;
    if (check_pathname_state(n->info_dir) != EEXIST)
      continue;

    bool already = false;
    for (st_waste *w = st_config_data->st_waste_folder_props_head;
         w != NULL; w = w->next_node)
    {
      if (strcmp(w->parent, n->trash_dir) == 0)
      {
        already = true;
        break;
      }
    }
    if (already)
      continue;

    struct stat tst;
    if (lstat(n->trash_dir, &tst) != 0)
      continue;

    st_waste *new_node = malloc(sizeof *new_node);
    if (!new_node)
      fatal_malloc();
    new_node->parent = strdup(n->trash_dir);
    new_node->files = strdup(n->files_dir);
    new_node->info = strdup(n->info_dir);
    new_node->len_files = strlen(new_node->files);
    new_node->len_info = strlen(new_node->info);
    new_node->media_root = (n->mount_path != NULL)
      ? strdup(n->mount_path) : NULL;
    new_node->removable = false;
    new_node->no_deposit = false;
    new_node->is_ficlone_fs = n->is_ficlone_fs;
    new_node->dev_num = tst.st_dev;
    new_node->next_node = NULL;

    st_waste *tail = st_config_data->st_waste_folder_props_head;
    if (tail == NULL)
    {
      new_node->prev_node = NULL;
      st_config_data->st_waste_folder_props_head = new_node;
    }
    else
    {
      while (tail->next_node != NULL)
        tail = tail->next_node;
      new_node->prev_node = tail;
      tail->next_node = new_node;
    }

    verbose_printf(1, "discovered topdir trash: %s\n", n->trash_dir);
  }

  free_mount_trash_list(mts);
  free(home_trash_dir);
}


/* With -l -v: show spec $topdir locations on eligible mounts that aren't in
 * the waste list yet — where a trash dir would be created on demand. */
static void
list_candidate_topdirs(const st_config *st_config_data,
                       const st_loc *st_location)
{
  char *home_trash_dir = home_trash_dir_for(st_location->home_dir);

  struct stat st;
  dev_t home_dev = 0;
  if (lstat(st_location->home_dir, &st) == 0)
    home_dev = st.st_dev;

  st_mount_trash *mts =
    build_mount_trash_list(st_config_data->uid, home_trash_dir, home_dev);

  bool have_header = false;
  for (st_mount_trash *n = mts; n != NULL; n = n->next)
  {
    bool listed = false;
    for (st_waste *w = st_config_data->st_waste_folder_props_head;
         w != NULL; w = w->next_node)
    {
      if (strcmp(w->parent, n->trash_dir) == 0)
      {
        listed = true;
        break;
      }
    }
    if (listed)
      continue;

    if (!have_header)
    {
      printf(_("\nCandidate trash locations (created when needed):\n"));
      have_header = true;
    }
    printf("  %s\n", n->trash_dir);
  }

  free_mount_trash_list(mts);
  free(home_trash_dir);
}


static const st_loc *
get_locations(const char *alt_config_file)
{
  const char mrl_file_basename[] = "mrl";
  const char purge_time_file_basename[] = "purge-time";

  static st_loc x;

  const char *enable_test = getenv(ENV_RMW_FAKE_HOME);
  if (enable_test != NULL)
  {
    static char s_xdg_data[PATH_MAX];
    static char s_xdg_config[PATH_MAX];
    sn_check(snprintf(s_xdg_data, sizeof s_xdg_data,
                      "%s/.local/share", enable_test), sizeof s_xdg_data);
    sn_check(snprintf(s_xdg_config, sizeof s_xdg_config,
                      "%s/.config", enable_test), sizeof s_xdg_config);
    setenv("HOME", enable_test, 1);
    setenv("XDG_DATA_HOME", s_xdg_data, 1);
    setenv("XDG_CONFIG_HOME", s_xdg_config, 1);
  }

  x.home_dir = getenv("HOME");
  if (x.home_dir == NULL)
    return NULL;

  if (enable_test)
    verbose_printf(1, "%s:%s\n", ENV_RMW_FAKE_HOME, enable_test);
  verbose_printf(1, "home_dir: %s\n", x.home_dir);

  static char s_data_dir[PATH_MAX];
  char *tmp = canfigger_data_dir(PACKAGE_STRING);
  if (!tmp)
    return NULL;
  sn_check(snprintf(s_data_dir, sizeof s_data_dir, "%s", tmp),
           sizeof s_data_dir);
  free(tmp);
  x.data_dir = s_data_dir;

  verbose_printf(1, "data_dir: %s\n", x.data_dir);

  char *default_config_file = canfigger_config_file("rmwrc");
  if (!default_config_file)
    return NULL;

  gchar *config_dir = g_path_get_dirname(default_config_file);

  verbose_printf(1, "config_dir: %s\n", config_dir);

  int p_state = check_pathname_state(config_dir);
  if (p_state == ENOENT)
  {
    if (!rmw_mkdir(config_dir))
      msg_success_mkdir(config_dir);
    else
    {
      msg_err_mkdir(config_dir, __func__);
      exit(errno);
    }
  }
  else if (p_state == -1)
    exit(p_state);

  g_free(config_dir);

  static char s_config_file[PATH_MAX];

  if (alt_config_file == NULL)
  {
    sn_check(snprintf
             (s_config_file, sizeof s_config_file, "%s", default_config_file),
             sizeof s_config_file);
    x.config_file = s_config_file;
  }
  else
    x.config_file = alt_config_file;

  free(default_config_file);

  verbose_printf(1, "config_file: %s\n", x.config_file);

  if ((p_state = check_pathname_state(x.config_file)) == ENOENT)
  {
    FILE *fp = fopen(x.config_file, "w");
    if (fp)
    {
      puts(_("Creating default configuration file:"));
      printf("  %s\n\n", x.config_file);

      print_config(fp);
      close_file(&fp, x.config_file, __func__);
    }
    else
    {
      open_err(x.config_file, __func__);
      puts(_("Unable to read or write a configuration file."));
      exit(errno);
    }
  }
  else if (p_state == -1)
    exit(p_state);

  static char s_mrl_file[PATH_MAX];
  char *m_tmp_str = join_paths(x.data_dir, mrl_file_basename);
  sn_check(snprintf(s_mrl_file, sizeof s_mrl_file, "%s", m_tmp_str),
           sizeof s_mrl_file);

  free(m_tmp_str);
  x.mrl_file = s_mrl_file;

  static char s_purge_time_file[PATH_MAX];
  m_tmp_str = join_paths(x.data_dir, purge_time_file_basename);
  sn_check(snprintf
           (s_purge_time_file, sizeof s_purge_time_file, "%s", m_tmp_str),
           sizeof s_purge_time_file);

  free(m_tmp_str);
  x.purge_time_file = s_purge_time_file;

  verbose_printf(1, "mrl_file: %s\n", x.mrl_file);
  verbose_printf(1, "purge_time_file: %s\n", x.purge_time_file);

  return &x;
}


int
main(const int argc, char *const argv[])
{
#ifdef ENABLE_NLS
  static char *locale_dir;
  locale_dir = getenv("RMW_LOCALEDIR");
  if (!locale_dir)
    locale_dir = LOCALEDIR;

  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE_STRING, locale_dir);
  textdomain(PACKAGE_STRING);
#endif

  rmw_options cli_user_options;
  init_rmw_options(&cli_user_options);
  parse_cli_options(argc, argv, &cli_user_options);

  verbose_printf(2, "PATH_MAX = %d\n", PATH_MAX);

#ifdef HAVE_FICLONE
  verbose_printf(1, "ficlone support: true\n");
#else
  verbose_printf(1, "ficlone support: false\n");
#endif

  const st_loc *st_location = get_locations(cli_user_options.alt_config_file);
  if (st_location == NULL)
  {
    diag(DIAG_ERR, "%s", _("while getting the path to your home directory\n"));
    return 1;
  }

  int p_state = 0;
  if ((p_state = check_pathname_state(st_location->data_dir)) == -1)
    exit(p_state);

  bool init_data_dir = (p_state == ENOENT);

  if (init_data_dir)
  {
    if (!rmw_mkdir(st_location->data_dir))
    {
      msg_success_mkdir(st_location->data_dir);
    }
    else
    {
      msg_err_mkdir(st_location->data_dir, __func__);
      printf(_("\
unable to create config and data directory\n\
Please check your configuration file and permissions\
\n\
\n"));
      printf(_("Unable to continue. Exiting...\n"));
      return errno;
    }
  }

  st_config st_config_data;
  init_config_data(&st_config_data);
  parse_config_file(&cli_user_options, &st_config_data, st_location);
  /* RMW_FAKE_HOME isolates $HOME but not real mount points, so
   * discovery would pick up host-machine topdir trashes during tests. */
  if (getenv(ENV_RMW_FAKE_HOME) == NULL)
    discover_existing_topdir_trashes(&st_config_data, st_location);

  if (cli_user_options.list)
  {
    list_waste_folders(st_config_data.st_waste_folder_props_head);
    /* gated like discovery above, so tests don't see host mounts */
    if (verbose && getenv(ENV_RMW_FAKE_HOME) == NULL)
      list_candidate_topdirs(&st_config_data, st_location);
    dispose_waste(st_config_data.st_waste_folder_props_head);
    return 0;
  }

  st_time st_time_var;
  init_time_vars(&st_time_var);

  int orphan_ctr = 0;
  if (cli_user_options.want_purge
      || is_time_to_purge(&st_time_var, st_location->purge_time_file))
  {
    if (!st_config_data.force_required || cli_user_options.force)
      purge(&st_config_data, &cli_user_options, &st_time_var, &orphan_ctr);
    else
      printf(_("purge has been skipped: use -f or --force\n"));
  }

  if (cli_user_options.want_orphan_chk)
  {
    orphan_maint(st_config_data.st_waste_folder_props_head, &st_time_var,
                 &orphan_ctr);
    dispose_waste(st_config_data.st_waste_folder_props_head);
    return 0;
  }

  if (cli_user_options.want_selection_menu)
  {
    int r = 0;
#if !defined DISABLE_CURSES
    r =
      restore_select(st_config_data.st_waste_folder_props_head, &st_time_var,
                     &cli_user_options);
#else
    printf("This rmw was built without menu support\n");
    r = 0;
#endif
    dispose_waste(st_config_data.st_waste_folder_props_head);
    return r;
  }

  if (cli_user_options.most_recent_list || cli_user_options.want_undo)
  {
    int res =
      process_mrl(st_config_data.st_waste_folder_props_head, &st_time_var,
                  st_location->mrl_file, &cli_user_options);
    dispose_waste(st_config_data.st_waste_folder_props_head);
    return res;
  }

  if (cli_user_options.want_restore)
  {
    int r = 0;
    /* subtract 1 from optind otherwise the first file in the list isn't
     * restored
     */
    int file_arg = 0;
    for (file_arg = optind - 1; file_arg < argc; file_arg++)
    {
      if (restore(argv[file_arg], &st_time_var, &cli_user_options,
                  st_config_data.st_waste_folder_props_head) != 0)
        r++;
    }

    dispose_waste(st_config_data.st_waste_folder_props_head);

    return r;
  }

  if (optind < argc)
  {
    int result = remove_to_waste(argc,
                                 argv,
                                 st_config_data.st_waste_folder_props_head,
                                 &st_time_var,
                                 st_location,
                                 &cli_user_options);

    if (result)
    {
      dispose_waste(st_config_data.st_waste_folder_props_head);
      /* Don't need to print any messages here. Any warnings or errors
       * should have been sent to stdout when they happened */
      return result;
    }
  }
  else if (!cli_user_options.want_purge &&
           !cli_user_options.want_empty_trash && !init_data_dir)
  {
    printf(_("Insufficient command line arguments given;\n\
Enter '%s -h' for more information\n"), argv[0]);
  }

  dispose_waste(st_config_data.st_waste_folder_props_head);

  return 0;
}
