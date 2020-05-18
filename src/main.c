/*
 * main.c
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 * Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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

#include "rmw.h"
#include "main.h"
#include "utils_rmw.h"
#include "restore_rmw.h"
#include "parse_cli_options.h"
#include "config_rmw.h"
#include "purging_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

/** Set when rmw is run with the --verbose option. Enables increased output
 * to stdout */
int verbose;

/*
 * Defined when `make check` is used to build rmw as a library for unit testing,
 * or if built as a library
 * main() will not be built into the library.
 */
#ifndef BUILD_LIBRARY

const char *HOMEDIR;

int
main (const int argc, char* const argv[])
{
  /* Make sure PATH_MAX has a sane value */
  if (PATH_MAX < 256)
  {
    print_msg_error ();
    fputs ("Invalid value for PATH_MAX\n", stderr);
    return 1;
  }

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  rmw_options cli_user_options;
  init_rmw_options (&cli_user_options);
  parse_cli_options (argc, argv, &cli_user_options);

  if (verbose > 1)
    printf ("PATH_MAX = %d\n", LEN_MAX_PATH - 1);

#ifdef DEBUG
verbose = 1;
#endif

  HOMEDIR = get_home_dir (STR_ENABLE_TEST);

  if (HOMEDIR != NULL)
    bufchk (HOMEDIR, LEN_MAX_PATH);
  else
  {
    print_msg_error ();
    fputs (_("while getting the path to your home directory\n"), stderr);
    return 1;
  }

  const char *data_dir = get_data_rmw_home_dir ();

  bool init_data_dir = (! exists (data_dir));

  if (init_data_dir)
  {
    if ((make_dir (data_dir) == MAKE_DIR_FAILURE))
    {
      print_msg_error ();
      printf (_("\
unable to create config and data directory\n\
Please check your configuration file and permissions\
\n\
\n"));
      printf (_("Unable to continue. Exiting...\n"));
      return MAKE_DIR_FAILURE;
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

  st_time st_time_var;
  init_time_vars (&st_time_var);

  if (cli_user_options.want_purge || is_time_to_purge(&st_time_var, data_dir))
  {
    if (!st_config_data.force_required || cli_user_options.force)
      purge (&st_config_data, &cli_user_options, &st_time_var);
    else
      printf (_("purge has been skipped: use -f or --force\n"));
  }

  if (cli_user_options.want_orphan_chk)
  {
    orphan_maint(st_config_data.st_waste_folder_props_head, &st_time_var);
    return 0;
  }

  if (cli_user_options.want_selection_menu)
  {
    int result = restore_select (st_config_data.st_waste_folder_props_head, &st_time_var);
    dispose_waste (st_config_data.st_waste_folder_props_head);
    return result;
  }

  const char *mrl_file = get_most_recent_list_filename (data_dir);

  if (cli_user_options.want_undo)
  {
    undo_last_rmw (st_config_data.st_waste_folder_props_head, &st_time_var, mrl_file);
    dispose_waste (st_config_data.st_waste_folder_props_head);
    return 0;
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
        restore_errors += Restore (argv[file_arg],
        st_config_data.st_waste_folder_props_head,
        &st_time_var));

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
      mrl_file);

    if (result > 1)
    {
      /* Don't need to print any messages here. Any warnings or errors
       * should have been sent to stdout when they happened */
      return result;
    }
  }
  else if (! cli_user_options.want_purge && ! cli_user_options.want_empty_trash && ! init_data_dir)
  {
    printf (_("Insufficient command line arguments given;\n\
Enter '%s -h' for more information\n"), argv[0]);
  }

  dispose_waste (st_config_data.st_waste_folder_props_head);

  return 0;
}
#else
  const char *HOMEDIR = "/home/andy";
#endif

/*
 * Returns a pointer to the home directory, or optionally sets an
 * alternate home directory (primarily useful for testing an app).
 * If an alternate home directory isn't needed, the argument passed
 * can be NULL instead of a string.
 */
const char *
get_home_dir (const char *alternate_home_dir)
{
  if (alternate_home_dir != NULL)
  {
    char *enable_test = getenv (alternate_home_dir);
    if (enable_test != NULL)
      return enable_test;
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
      snprintf (combined_path, sizeof combined_path, "%s%s", _drive, _path);
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

  if (getenv (STR_ENABLE_TEST) != NULL ||
      (xdg_data_home == NULL && getenv (STR_ENABLE_TEST) == NULL))
  {
    int req_len = multi_strlen (HOMEDIR, rel_default, NULL);
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    sprintf (data_rmw_home, "%s%s", HOMEDIR, rel_default);
    return ptr;
  }

  int req_len = multi_strlen (xdg_data_home, "/rmw", NULL);
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  sprintf (data_rmw_home, "%s/rmw", xdg_data_home);
  ptr = &data_rmw_home[0];
  return ptr;
}


const char*
get_most_recent_list_filename (const char* data_dir)
{
  const char rel_most_recent_list_filename[] = "mrl";
  int req_len = multi_strlen (data_dir, "/", rel_most_recent_list_filename, NULL);
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  static char mrl_file[LEN_MAX_PATH];
  sprintf (mrl_file, "%s/%s", data_dir, rel_most_recent_list_filename);
  static const char *ptr;
  ptr = &mrl_file[0];
  return ptr;
}


int
remove_to_waste (
  const int argc,
  char* const argv[],
  st_waste *waste_head,
  st_time *st_time_var,
  const char *mrl_file)
{
  rmw_target st_file_properties;

  st_removed *confirmed_removals_list = NULL;
  st_removed *confirmed_removals_list_head = NULL;

  static int main_error_ctr;
  int removed_files_ctr = 0;
  int file_arg;
  for (file_arg = optind; file_arg < argc; file_arg++)
  {
    /* leave "/" or "\" alone */
    if (strcmp (argv[file_arg], "/") == 0 || strcmp (argv[file_arg], "/") == 0)
    {
      puts (_("The Easter Bunny says, \"Hello, world.\""));
      continue;
    }

    bufchk (argv[file_arg], LEN_MAX_PATH);
    st_file_properties.main_argv = argv[file_arg];

    static struct stat st_main_argv_statistics;
    if (!lstat (st_file_properties.main_argv, &st_main_argv_statistics))
    {
      main_error_ctr = resolve_path (st_file_properties.main_argv, st_file_properties.real_path);
      if (main_error_ctr == 1)
        continue;
      else if (main_error_ctr > 1)
        break;
    }
    else
    {
      printf (_("File not found: '%s'\n"), st_file_properties.main_argv);
      continue;
    }

    /**
     * Make some variables -
     * get ready for the ReMoval
     */

    bool waste_folder_on_same_filesystem = 0;

    st_file_properties.base_name = basename ((char*)st_file_properties.main_argv);

    /**
     * cycle through wasteDirs to see which one matches
     * device number of file.main_argv. Once found, the ReMoval
     * happens (provided all the tests are passed.
     */
    st_waste *waste_curr = waste_head;
    while (waste_curr != NULL)
    {
      if (waste_curr->dev_num == st_main_argv_statistics.st_dev)
      {
        sprintf (st_file_properties.waste_dest_name, "%s%s",
                  waste_curr->files, st_file_properties.base_name);

        /* If a duplicate file exists
         */
        if ((st_file_properties.is_duplicate = exists (st_file_properties.waste_dest_name)))
        {
          // append a time string
          int req_len = multi_strlen (st_file_properties.waste_dest_name, st_time_var->suffix_added_dup_exists, NULL) + 1;
          bufchk_len (req_len, sizeof st_file_properties.waste_dest_name, __func__, __LINE__);
          strcat (st_file_properties.waste_dest_name, st_time_var->suffix_added_dup_exists);
        }

        if (rename (st_file_properties.main_argv, st_file_properties.waste_dest_name) == 0)
        {
          if (verbose)
            printf ("'%s' -> '%s'\n", st_file_properties.main_argv, st_file_properties.waste_dest_name);

          removed_files_ctr++;

          if (!create_trashinfo (&st_file_properties, waste_curr, st_time_var))
          {
            confirmed_removals_list = add_removal (confirmed_removals_list, st_file_properties.waste_dest_name);
            if (confirmed_removals_list_head == NULL)
              confirmed_removals_list_head = confirmed_removals_list;
          }
        }
        else
          msg_err_rename (st_file_properties.main_argv,
                          st_file_properties.waste_dest_name,
                          __func__, __LINE__);

    /**
     * If we get to this point, it means a WASTE folder was found
     * that matches the file system that file->main_argv was on.
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
      printf (_("No suitable filesystem found for \"%s\"\n"), st_file_properties.main_argv);
    }
  }

  if (confirmed_removals_list_head != NULL)
  {
    create_undo_file (confirmed_removals_list_head, mrl_file);
    dispose_removed (confirmed_removals_list_head);
  }

  printf (ngettext ("%d file was removed to the waste folder", "%d files were removed to the waste folder",
          removed_files_ctr), removed_files_ctr);
  printf ("\n");

  return main_error_ctr;
}


void
list_waste_folders (st_waste *waste_head)
{
  st_waste *waste_curr = waste_head;
  while (waste_curr != NULL)
  {
    show_folder_line (waste_curr->parent, waste_curr->removable);
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
  bufchk (file, sizeof removals->file);
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
