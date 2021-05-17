/*
 * main.c
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 * Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
 * Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md
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

#include "globals.h"
#include "main.h"
#include "utils_rmw.h"
#include "bst.h"
#include "restore_rmw.h"
#include "config_rmw.h"
#include "purging_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

const char *mrl_is_empty = "[Empty]\n";
/*
 * Defined when `make check` is used to build rmw as a library for unit testing,
 * or if built as a library
 * main() will not be built into the library.
 */
#ifndef TEST_LIB

static const char*
get_most_recent_list_filename (const char* data_dir)
{
  const char rel_most_recent_list_filename[] = "mrl";
  int req_len = multi_strlen (data_dir, "/", rel_most_recent_list_filename, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  static char mrl_file[LEN_MAX_PATH];
  sprintf (mrl_file, "%s/%s", data_dir, rel_most_recent_list_filename);
  static const char *ptr;
  ptr = &mrl_file[0];
  if (verbose)
    printf ("most recent list (mrl file): %s\n", mrl_file);
  return ptr;
}

static char*
get_mrl_contents (const char *mrl_file)
{
  char *contents = NULL;
  FILE *fd;
  fd = fopen (mrl_file, "r");

  if (fd == NULL)
  {
    contents = (char*)mrl_is_empty;
    return contents;
  }

  fseek (fd, 0, SEEK_END); // move to the end of the file so we can use ftell()
  const int f_size = ftell (fd); // Get the size of the file
  rewind (fd);

  contents = calloc (1, f_size + 1);
  chk_malloc (contents, __func__, __LINE__);
  int n_items = fread (contents, 1, f_size + 1, fd);
  if (n_items != f_size)
  {
    print_msg_warn ();
    fprintf (stdout, "ftell() returned %d, but fread() returned %d", f_size, n_items);
  }
  if (feof (fd) == 0)
  {
    print_msg_error ();
    fprintf (stderr, "while reading %s\n", mrl_file);
    clearerr (fd);
  }
  close_file (fd, mrl_file, __func__);
  return contents;
}


static int
process_mrl (st_waste *waste_head,
    st_time *st_time_var,
    const char *mrl_file,
    rmw_options *cli_user_options)
{
  char *mrl_contents = get_mrl_contents (mrl_file);
  int res = 0;
  if (mrl_contents != NULL)
  {
    if (!strcmp (mrl_contents, mrl_is_empty))
      printf ("There are no items in the list - please check back later.\n");
    else if (cli_user_options->most_recent_list)
    {
      printf ("%s", mrl_contents);
      if (cli_user_options->want_undo)
        puts (_("Skipping --undo-last because --most-recent-list was requested"));
    }
    else
      res = undo_last_rmw (st_time_var, mrl_file, cli_user_options, mrl_contents, waste_head);

    if (mrl_contents != NULL && mrl_contents != mrl_is_empty)
    {
      free (mrl_contents);
    }
    return res;
  }
  fprintf (stderr, "A 'NULL' was returned when trying to read the list\n");
  return -1;
}

int
main (const int argc, char* const argv[])
{
  #ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  #endif

  rmw_options cli_user_options;
  init_rmw_options (&cli_user_options);
  parse_cli_options (argc, argv, &cli_user_options);

  if (verbose > 1)
    printf ("PATH_MAX = %d\n", LEN_MAX_PATH - 1);

  HOMEDIR = get_home_dir ();

  if (HOMEDIR != NULL)
    bufchk_len (strlen (HOMEDIR) + 1, LEN_MAX_PATH, __func__, __LINE__);
  else
  {
    print_msg_error ();
    fputs (_("while getting the path to your home directory\n"), stderr);
    return 1;
  }

  const char *data_dir = get_data_rmw_home_dir ();

  bool init_data_dir = !exists (data_dir);

  if (init_data_dir)
  {
    if (!rmw_mkdir (data_dir, S_IRWXU))
    {
      msg_success_mkdir (data_dir);
    }
    else
    {
      msg_err_mkdir (data_dir, __func__);
      printf (_("\
unable to create config and data directory\n\
Please check your configuration file and permissions\
\n\
\n"));
      printf (_("Unable to continue. Exiting...\n"));
      return errno;
    }
  }

  st_config st_config_data;
  init_config_data (&st_config_data);
  parse_config_file (&cli_user_options, &st_config_data);

  if (cli_user_options.list)
  {
    list_waste_folders (st_config_data.st_waste_folder_props_head);
    return 0;
  }

  init_trashinfo_spec ();

  st_time st_time_var;
  init_time_vars (&st_time_var);

  int orphan_ctr = 0;
  if (cli_user_options.want_purge || is_time_to_purge(&st_time_var, data_dir))
  {
    if (!st_config_data.force_required || cli_user_options.force)
      purge (&st_config_data, &cli_user_options, &st_time_var, &orphan_ctr);
    else
      printf (_("purge has been skipped: use -f or --force\n"));
  }

  if (cli_user_options.want_orphan_chk)
  {
    orphan_maint(st_config_data.st_waste_folder_props_head, &st_time_var, &orphan_ctr);
    dispose_waste (st_config_data.st_waste_folder_props_head);
    return 0;
  }

  if (cli_user_options.want_selection_menu)
  {
    int result = restore_select (st_config_data.st_waste_folder_props_head, &st_time_var, &cli_user_options);
    dispose_waste (st_config_data.st_waste_folder_props_head);
    return result;
  }

  const char *mrl_file = get_most_recent_list_filename (data_dir);

  if (cli_user_options.most_recent_list || cli_user_options.want_undo)
  {
    int res = process_mrl (st_config_data.st_waste_folder_props_head, &st_time_var, mrl_file, &cli_user_options);
    dispose_waste (st_config_data.st_waste_folder_props_head);
    return res;
  }


  if (cli_user_options.want_restore)
  {
    int restore_errors = 0;
    /* subtract 1 from optind otherwise the first file in the list isn't
     * restored
     */
    int file_arg = 0;
    for (file_arg = optind - 1; file_arg < argc; file_arg++)
      msg_warn_restore(
        restore_errors += restore (argv[file_arg],
        &st_time_var,
        &cli_user_options,
        st_config_data.st_waste_folder_props_head));

    dispose_waste (st_config_data.st_waste_folder_props_head);

    return restore_errors;
  }

  if (optind < argc)
  {
    int result = remove_to_waste (
      argc,
      argv,
      st_config_data.st_waste_folder_props_head,
      &st_time_var,
      mrl_file,
      &cli_user_options);

    if (result > 1)
    {
      dispose_waste (st_config_data.st_waste_folder_props_head);
      /* Don't need to print any messages here. Any warnings or errors
       * should have been sent to stdout when they happened */
      return result;
    }
  }
  else if (! cli_user_options.want_purge &&
          ! cli_user_options.want_empty_trash &&
          ! init_data_dir)
  {
    printf (_("Insufficient command line arguments given;\n\
Enter '%s -h' for more information\n"), argv[0]);
  }

  dispose_waste (st_config_data.st_waste_folder_props_head);

  return 0;
}
#endif

/*
 * Returns a pointer to the home directory, or optionally sets an
 * alternate home directory (primarily useful for testing an app).
 * If an alternate home directory isn't needed, the argument passed
 * can be NULL instead of a string.
 */
const char *
get_home_dir (void)
{
  const char *alternate_home_dir = NULL;
  const char *deprecated_env = getenv (ENV_TEST_HOME);
  if (deprecated_env != NULL)
  {
    print_msg_warn ();
    printf ("'%s' will be deprecated, use '%s' instead\n", ENV_TEST_HOME, ENV_RMW_FAKE_HOME);
    alternate_home_dir = deprecated_env;
  }

  if (alternate_home_dir == NULL)
    alternate_home_dir = getenv (ENV_RMW_FAKE_HOME);

  if (alternate_home_dir != NULL)
  {
    if (verbose)
      printf ("%s:%s\n", ENV_RMW_FAKE_HOME, alternate_home_dir);
    return alternate_home_dir;
  }

  char *_homedir;

  #ifndef WIN32
    _homedir = getenv ("HOME");
  #else
    char *_drive = getenv ("HOMEDRIVE");
    char *_path = getenv ("HOMEPATH");

    if (_drive != NULL && _path != NULL)
    {
      static char combined_path[LEN_MAX_PATH];
      sprintf (combined_path, "%s%s", _drive, _path);
      _homedir = &combined_path[0];
    }
    else
      _homedir = NULL;
  #endif

  return _homedir;
}


const char*
get_data_rmw_home_dir (void)
{
  const char rel_default[] = "/.local/share/rmw";

  const char *xdg_data_home = getenv ("XDG_DATA_HOME");

  static char data_rmw_home[LEN_MAX_PATH];
  static const char *ptr = &data_rmw_home[0];
  const char *enable_test = getenv (ENV_RMW_FAKE_HOME);

  if (enable_test != NULL ||
      (xdg_data_home == NULL && enable_test == NULL))
  {
    int req_len = multi_strlen (HOMEDIR, rel_default, NULL) + 1;
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    sprintf (data_rmw_home, "%s%s", HOMEDIR, rel_default);
    return ptr;
  }

  int req_len = multi_strlen (xdg_data_home, "/rmw", NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  sprintf (data_rmw_home, "%s/rmw", xdg_data_home);
  ptr = &data_rmw_home[0];
  return ptr;
}

int
remove_to_waste (
  const int argc,
  char* const argv[],
  st_waste *waste_head,
  st_time *st_time_var,
  const char *mrl_file,
  const rmw_options * cli_user_options)
{
  rmw_target st_target;

  st_removed *confirmed_removals_list = NULL;
  st_removed *confirmed_removals_list_head = NULL;

  int n_err = 0;
  int removed_files_ctr = 0;
  int file_arg;
  for (file_arg = optind; file_arg < argc; file_arg++)
  {
    int orig_req_len = strlen (argv[file_arg]) + 1;
    bufchk_len (orig_req_len, LEN_MAX_PATH, __func__, __LINE__);
    st_target.orig = argv[file_arg];

    bufchk_len (orig_req_len, LEN_MAX_PATH, __func__, __LINE__);
    char tmp[orig_req_len];
    strcpy (tmp, st_target.orig);
    st_target.base_name = basename (tmp);

    if (isdotdir (st_target.base_name))
    {
      printf ("refusing to ReMove '.' or '..' directory: skipping '%s'\n", argv[file_arg]);
      continue;
    }

    /* leave "/" or "\" alone */
    if (strcmp (argv[file_arg], "/") == 0 || strcmp (argv[file_arg], "/") == 0)
    {
      puts (_("\n\
Your single slash has been ignored. You walk to the market\n\
in the town square and purchase a Spear of Destiny. You walk to\n\
the edge of the forest and find your enemy. You attack, causing\n\
damage of 5000 hp. You feel satisfied.\n"));
      continue;
    }

    struct stat st_orig;
    if (!lstat (st_target.orig, &st_orig))
    {
      st_target.real_path = resolve_path (st_target.orig, st_target.base_name);
      if (st_target.real_path == NULL)
      {
        n_err++;
        if (n_err > 1)
          continue;
      }
    }
    else
    {
      printf (_("File not found: '%s'\n"), st_target.orig);
      continue;
    }

    /* Make sure the file isn't a waste folder or a file within a waste folder */
    bool is_protected = 0;
    st_waste *waste_curr = waste_head;
    while (waste_curr != NULL)
    {
      if (strncmp (waste_curr->parent, st_target.real_path, strlen (waste_curr->parent)) == 0)
      {
        print_msg_warn ();
        printf (_("%s resides within a waste folder and has been ignored\n"), st_target.orig);
        is_protected = 1;
        break;
      }
      waste_curr = waste_curr->next_node;
    }
    if (is_protected)
    {
      free (st_target.real_path);
      continue;
    }

    /**
     * Make some variables -
     * get ready for the ReMoval
     */

    bool waste_folder_on_same_filesystem = 0;

    /**
     * cycle through wasteDirs to see which one matches
     * device number of file.orig. Once found, the ReMoval
     * happens (provided all the tests are passed.
     */
    waste_curr = waste_head;
    while (waste_curr != NULL)
    {
      if (waste_curr->dev_num == st_orig.st_dev)
      {
        int req_len = multi_strlen (st_target.base_name, "/", NULL) + waste_curr->len_files + 1;
        bufchk_len (req_len, sizeof st_target.waste_dest_name, __func__, __LINE__);
        sprintf (st_target.waste_dest_name, "%s/%s",
                  waste_curr->files, st_target.base_name);

        /* If a duplicate file exists
         */
        if ((st_target.is_duplicate = exists (st_target.waste_dest_name)))
        {
          // append a time string
          bufchk_len (strlen (st_target.waste_dest_name) + LEN_MAX_TIME_STR_SUFFIX, sizeof st_target.waste_dest_name, __func__, __LINE__);
          strcat (st_target.waste_dest_name, st_time_var->suffix_added_dup_exists);
        }

        int r_result = 0;
        if (cli_user_options->want_dry_run == false)
          r_result = rename (st_target.orig, st_target.waste_dest_name);

        if (r_result == 0)
        {
          if (verbose)
            printf ("'%s' -> '%s'\n", st_target.orig, st_target.waste_dest_name);

          removed_files_ctr++;

          if (cli_user_options->want_dry_run == false)
            if (!create_trashinfo (&st_target, waste_curr, st_time_var))
            {
              free (st_target.real_path);
              confirmed_removals_list = add_removal (confirmed_removals_list, st_target.waste_dest_name);
              if (confirmed_removals_list_head == NULL)
                confirmed_removals_list_head = confirmed_removals_list;
            }
        }
        else
          msg_err_rename (st_target.orig,
                          st_target.waste_dest_name,
                          __func__, __LINE__);

    /**
     * If we get to this point, it means a WASTE folder was found
     * that matches the file system that file->orig was on.
     * Setting match to 1 and breaking from the for loop
     */
        waste_folder_on_same_filesystem = 1;
        break;
      }

      /* If the file didn't match with a waste folder on the same filesystem,
       * try the next waste folder */
      waste_curr = waste_curr->next_node;
    }

    if (!waste_folder_on_same_filesystem)
    {
      print_msg_warn ();
      printf (_("No suitable filesystem found for \"%s\"\n"), st_target.orig);
    }
  }

  if (confirmed_removals_list_head != NULL)
  {
    create_undo_file (confirmed_removals_list_head, mrl_file);
    dispose_removed (confirmed_removals_list_head);
  }

  printf (ngettext ("%d item was removed to the waste folder", "%d items were removed to the waste folder",
          removed_files_ctr), removed_files_ctr);
  printf ("\n");

  return n_err;
}


void
list_waste_folders (st_waste *waste_head)
{
  // Directories that are not on attached medium are not included in
  // the waste linked list, so we can just assign true here.
  const bool is_attached = true;

  st_waste *waste_curr = waste_head;
  while (waste_curr != NULL)
  {
    show_folder_line (waste_curr->parent, waste_curr->removable, is_attached);
    waste_curr = waste_curr->next_node;
  }

  dispose_waste (waste_head);
  return;
}


/*!
 * When a file has been successfully rmw'ed, add the filename to
 * a linked list.
 */
st_removed*
add_removal (st_removed *removals, const char *file)
{
  if (removals == NULL)
  {
    st_removed *temp_node = (st_removed*)malloc (sizeof (st_removed));
    chk_malloc (temp_node, __func__, __LINE__);
    removals = temp_node;
  }
  else
  {
    removals->next_node = (st_removed*)malloc (sizeof (st_removed));
    chk_malloc (removals->next_node, __func__, __LINE__);
    removals = removals->next_node;
  }
  removals->next_node = NULL;
  bufchk_len (strlen (file) + 1, sizeof removals->file, __func__, __LINE__);
  strcpy (removals->file, file);
  return removals;
}


/*!
 * recursively remove all nodes of an object of type @ref st_removed
 */
void
dispose_removed (st_removed *node)
{
  if (node != NULL)
  {
    dispose_removed (node->next_node);
    free (node);
  }

  return;
}


/*!
 * Create a new undo_file (lastrmw)
 */
void
create_undo_file (st_removed *removals_head, const char* mrl_file)
{
  FILE *fd = fopen (mrl_file, "w");
  if (fd)
  {
    st_removed *st_removals_list = removals_head;
    while (st_removals_list != NULL)
    {
      fprintf (fd, "%s\n", st_removals_list->file);
      st_removals_list = st_removals_list->next_node;
    }
    close_file (fd, mrl_file, __func__);
  }
  else
    open_err (mrl_file, __func__);

  return;
}

