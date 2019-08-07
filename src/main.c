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

/*
 * Defined when `make check` is used to build rmw as a library for unit testing,
 * or if built as a library
 * main() will not be built into the library.
 */
#ifndef BUILD_LIBRARY

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

  HOMEDIR = get_homedir ("RMWTEST_HOME");

  if (HOMEDIR != NULL)
    bufchk (HOMEDIR, LEN_MAX_PATH);
  else
  {
    print_msg_error ();
    fputs (_("while getting the path to your home directory\n"), stderr);
    return 1;
  }

  int req_len = multi_strlen (HOMEDIR, DATA_DIR, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char data_dir[LEN_MAX_PATH];
  sprintf (data_dir, "%s%s", HOMEDIR, DATA_DIR);
  int created_data_dir = make_dir (data_dir);

  if (created_data_dir == MAKE_DIR_FAILURE)
  {
    print_msg_error ();
    printf (_("unable to create config and data directory\n\
Please check your configuration file and permissions\n\n"));
    printf (_("Unable to continue. Exiting...\n"));
    return MAKE_DIR_FAILURE;
  }

  /*
   * Using FIRST_RUN makes the code a little more clear when it's used
   * later. make_dir() will always return MAKE_DIR_SUCCESS if successful,
   * but if make_dir() is used in other areas of the program, it's not
   * necessarily the FIRST_RUN.
   */
  created_data_dir = (created_data_dir == MAKE_DIR_SUCCESS) ? FIRST_RUN : 0;

  st_config st_config_data;
  init_config_data (&st_config_data);
  get_config_data (&cli_user_options, &st_config_data);

  if (cli_user_options.list)
  {
    list_waste_folders (st_config_data.st_waste_folder_props_head);
    return 0;
  }

  st_time st_time_var;
  init_time_vars (&st_time_var);

  if (cli_user_options.want_purge || is_time_to_purge(&st_time_var))
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

  if (cli_user_options.want_undo)
  {
    undo_last_rmw (st_config_data.st_waste_folder_props_head, &st_time_var);
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
    int result = remove_to_waste (argc, argv, st_config_data.st_waste_folder_props_head, &st_time_var);
    if (result > 1)
    {
      /* Don't need to print any messages here. Any warnings or errors
       * should have been sent to stdout when they happened */
      return result;
    }
  }
  else if (!cli_user_options.want_purge && !cli_user_options.want_empty_trash && created_data_dir != FIRST_RUN)
  {
    printf (_("Insufficient command line arguments given;\n\
Enter '%s -h' for more information\n"), argv[0]);
  }

  dispose_waste (st_config_data.st_waste_folder_props_head);

  return 0;
}
#endif

/* returns a pointer to the home directory, or optionally sets an
 * alternate home directory (primarily useful for testing an app).
 * If an alternate home directory isn't needed, the argument passed
 * can be NULL instead of a string.
 */
const char *
get_homedir (const char *alternate_homedir)
{
  if (alternate_homedir != NULL)
  {
    char *enable_test = getenv (alternate_homedir);
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

  if (_homedir == NULL)
    return NULL;

  return _homedir;
}

int
remove_to_waste (
  const int argc,
  char* const argv[],
  st_waste *waste_head,
  st_time *st_time_var)
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
    create_undo_file (confirmed_removals_list_head);
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
    printf ("%s", waste_curr->parent);
    if (waste_curr->removable && verbose)
    {
    /*
     * These lines are separated to ease translation
     *
     */

      printf (" (");
      printf (_("removable, "));
      /* TRANSLATORS: context - "a mounted device or filesystem is presently attached or mounted" */
      printf (_("attached"));
      printf (")");
    }

    printf ("\n");

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
 * @param[out] node the node to be removed
 * @return void
 * @see add_removal
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
 * @param[in] removals_head the first node in the removals list
 * @return void
 * @see cli_user_options.want_undo_rmw
 * @see add_removal
 */
void
create_undo_file (st_removed *removals_head)
{
  int req_len = multi_strlen (HOMEDIR, UNDO_FILE, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char undo_path[req_len];
  sprintf (undo_path, "%s%s", HOMEDIR, UNDO_FILE);
  FILE *undo_file_ptr = fopen (undo_path, "w");
  if (undo_file_ptr != NULL)
  {
    st_removed *st_removals_list = removals_head;
    while (st_removals_list != NULL)
    {
      fprintf (undo_file_ptr, "%s\n", st_removals_list->file);
      st_removals_list = st_removals_list->next_node;
    }
    close_file (undo_file_ptr, undo_path, __func__);
  }
  else
    open_err (undo_path, __func__);

  return;
}
