/*
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
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>

#include "utils_rmw.h"
#include "restore_rmw.h"
#include "trivial_rmw.h"
#include "config_rmw.h"
#include "trashinfo_rmw.h"
#include "purging_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

/*
 * These variables are used by only a few functions and don't need to be global.
 * Will use "extern" to declare them where they are needed.
 *
 */

/*! Read from the user's config file */
int purge_after = 0;

/*! Set from the command line and optionally from the user's config file */
bool force_required = 0;

/*
 * Defined when `make check` is used to build rmw as a library for unit testing,
 * or if built as a library
 * main() will not be built into the library.
 */
#ifndef BUILD_LIBRARY

/*!
 * The "main" part of rmw
 * @param[in] argc The number of paramaters given to rmw at run-time
 * @param[in] argv An array of arguments given to rmw at run-time
 * @return an error code of type int, usually 0 because this main function hardly ever segfaults
 */
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

  const char *const short_options = "hvc:goz:lsuwVfeir";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"orphaned", 0, NULL, 'o'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {"interactive", 0, NULL, 'i'},
    {"recurse", 0, NULL, 'r'},
    {"force", 0, NULL, 'f'},
    {"empty", 0, NULL, 'e'},
    {NULL, 0, NULL, 0}
  };

  short int next_option = 0;

  verbose = 0; /* already declared, a global */
  rmw_options cli_user_options;
  rmw_option_init (&cli_user_options);

  do
  {
    next_option = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (next_option)
    {
    case 'h':
      print_usage ();
      exit (0);
    case 'v':
      verbose++;
      break;
    case 'c':
      cli_user_options.alt_config = optarg;
      break;
    case 'l':
      cli_user_options.list = true;
      break;
    case 'g':
      cli_user_options.want_purge = true;
      break;
    case 'o':
      cli_user_options.want_orphan_chk = 1;
      break;
    case 'z':
      cli_user_options.want_restore = true;
      break;
    case 's':
      cli_user_options.want_selection_menu = 1;
      break;
    case 'u':
      cli_user_options.want_undo = 1;
      break;
    case 'w':
      warranty ();
      break;
    case 'V':
      version ();
      break;
    case 'i':
      printf (_("-i / --interactive: not implemented\n"));
      break;
    case 'r':
      printf (_("-r / --recurse: not implemented\n"));
      break;
    case 'f':
      if (cli_user_options.force < 2) /* This doesn't need to go higher than 2 */
        cli_user_options.force++;
      break;
    case 'e':
      cli_user_options.want_empty_trash = true;
      cli_user_options.want_purge = true;
      break;
    case '?':
      print_usage ();
      exit (0);
    case -1:
      break;
    default:
      abort ();
    }

  }
  while (next_option != -1);

  if (verbose > 1)
    printf ("PATH_MAX = %d\n", MP - 1);

#ifdef DEBUG
verbose = 1;
#endif

    /* rmw doesn't work on Windows yet */
    /*! @bug <a href="https://github.com/theimpossibleastronaut/rmw/issues/71">Running and building rmw on Windows</a> */
  #ifndef WIN32
    HOMEDIR = getenv ("HOME");
  #else
    HOMEDIR = getenv ("LOCALAPPDATA");
  #endif

  if (HOMEDIR != NULL)
    bufchk (HOMEDIR, MP);
  else
  {
    print_msg_error ();
    /* FIXME: Perhaps there should be an option in the config file so a
     * user can specify a home directory, and pass the $HOME variable
     */
    fputs (_("while getting the path to your home directory\n"), stderr);
    return 1;
  }

  int req_len = multi_strlen (HOMEDIR, DATA_DIR, NULL) + 1;
  bufchk_len (req_len, MP, __func__, __LINE__);
  char data_dir[req_len];
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

  st_waste *waste_head;
  waste_head = get_config_data (&cli_user_options);

  st_waste *waste_curr = waste_head;

  if (cli_user_options.list)
  {
    list_waste_folders (waste_head);
    return 0;
  }

  st_time st_time_var;
  time_var_init (&st_time_var);

  if (cli_user_options.want_purge || is_time_to_purge(&st_time_var))
  {
    if (!force_required || cli_user_options.force)
      purge (waste_curr, &cli_user_options, &st_time_var);
    else
      printf (_("purge has been skipped: use -f or --force\n"));
  }

  if (cli_user_options.want_orphan_chk)
  {
    waste_curr = waste_head;
    orphan_maint(waste_curr, &st_time_var);
    return 0;
  }

  if (cli_user_options.want_selection_menu)
  {
    waste_curr = waste_head;
    int result = restore_select (waste_curr, &st_time_var);
    dispose_waste (waste_head);
    return result;
  }

  if (cli_user_options.want_undo)
  {
    waste_curr = waste_head;
    undo_last_rmw (waste_curr, &st_time_var);
    dispose_waste (waste_head);
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
    {
      waste_curr = waste_head;
      msg_warn_restore(restore_errors += Restore (argv[file_arg], waste_curr, &st_time_var));
    }

    return restore_errors;
  }

  if (optind < argc) /* FIXME: shouldn't this be "else if"? */
  {
    int result = send_to_waste (argc, argv, waste_head, &st_time_var);
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

  dispose_waste (waste_head);

  return 0;
}
#endif


int
send_to_waste (
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

    bufchk (argv[file_arg], MP);
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
          bufchk_len (req_len, MP, __func__, __LINE__);
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


void
rmw_option_init (rmw_options *options)
{
  options->want_restore = false;
  options->want_purge = false;
  options->want_empty_trash = false;
  options->want_orphan_chk = false;
  options->want_selection_menu = false;
  options->want_undo = false;
  options->force = 0;
  options->list = false;
  options->alt_config = NULL;

}


/*!
 * Called in @ref main() to determine whether or not @ref purge() was run today, reads
 * and writes to the 'lastpurge` file. If it hasn't been run today, the
 * current day will be written. If 'lastpurge' doesn't exist, it gets
 * created.
 * @return FALSE if the day is the same (don't purge); otherwise TRUE and
 * writes the current day to 'lastpurge'
 */
bool
is_time_to_purge (st_time *st_time_var)
{
  const int BUF_TIME = 80;
  int req_len = multi_strlen (HOMEDIR, PURGE_DAY_FILE, NULL) + 1;
  bufchk_len (req_len, MP, __func__, __LINE__);
  char file_lastpurge[req_len];
  snprintf (file_lastpurge, req_len, "%s%s", HOMEDIR, PURGE_DAY_FILE);

  FILE *fp = fopen (file_lastpurge, "r");
  bool init = (fp != NULL);
  if (fp)
  {
    char time_prev[BUF_TIME];

    if (fgets (time_prev, BUF_TIME, fp) == NULL)
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
      return FALSE;
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
 * When a file has been successfully rmw'ed, add the filename to
 * a linked list.
 *
 * @param[out] removals linked lists of files that have been succesfully rmw'ed
 * @param[in] file file that has been successfully rmw'ed
 * @see cli_user_options.want_undo_rmw
 * @return pointer to node in @ref st_removed type struct
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
  bufchk (file, MP);
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
  char undo_path[MP];
  bufchk (UNDO_FILE, MP - strlen (HOMEDIR));
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


void
time_var_init (st_time *st_time_var)
{
  st_time_var->now = time (NULL);

  set_which_deletion_date (st_time_var, LEN_DELETION_DATE);

  set_time_string (
    st_time_var->deletion_date,
    LEN_DELETION_DATE,
    st_time_var->t_fmt,
    st_time_var->now);

  set_time_string (
    st_time_var->suffix_added_dup_exists,
    LEN_TIME_STR_SUFFIX,
    "_%H%M%S-%y%m%d",
    st_time_var->now);

  return;
}


/*!
 * Returns a formatted string based on whether or not
 * a fake year is requested at runtime. If a fake-year is not requested,
 * the returned string will be based on the local-time of the user's system.
 * @param[in] st_time_var structure holding time-related variables.
 * @param[in] len The length required for the formatted string.
 * @return void
 */
void
set_which_deletion_date (st_time *st_time_var, const int len)
{
  if  (getenv ("RMWTRASH") == NULL ||
      (getenv ("RMWTRASH") != NULL && strcmp (getenv ("RMWTRASH"), "fake-year") != 0))
    snprintf (st_time_var->t_fmt, len, "%s", "%FT%T");
  else
  {
    printf ("  :test mode: Using fake year\n");
    snprintf (st_time_var->t_fmt, len, "%s", "1999-%m-%dT%T");
  }
  return;
}


/*!
 * Assigns a time string to *tm_str based on the format requested
 * @param[out] tm_str assigned a new value based on format
 * @param[in] len the max length of string to be allocated
 * @param[in] format the format of the string
 * @param[in] time_t_now The current time returned by a call to time(NULL).
 * @return void
 */
void
set_time_string (char *tm_str, const int len, const char *format, time_t time_t_now)
{
  struct tm *time_ptr;
  time_ptr = localtime (&time_t_now);
  strftime (tm_str, len, format, time_ptr);
  trim_white_space (tm_str);
  bufchk (tm_str, len);
}

