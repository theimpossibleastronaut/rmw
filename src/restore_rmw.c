/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2020  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
  #include "globals.h"
#endif

#include "parse_cli_options.h"
#include "config_rmw.h"
#include "trashinfo_rmw.h"
#include "restore_rmw.h"
#include "utils_rmw.h"
#include "messages_rmw.h"
#include "bst.h"

#ifndef TEST_LIB

/**
 * restores a file that was previously moved via rmw.
 *
 * FIXME: The name of the first paramater needs changing. It's not really
 * argv but the name of a file selected for restoration. Only in some cases
 * will it really be argv.
 */
int
restore (const char *argv, st_time *st_time_var, const rmw_options * cli_user_options)
{
  static struct restore
  {
    char *base_name;
    char relative_path[LEN_MAX_PATH];
    char dest[LEN_MAX_PATH];
    char info[LEN_MAX_PATH];
    char relative_info_path[9];
  } file;

  sprintf (file.relative_info_path, "%s", "../info/");

  bufchk (argv, LEN_MAX_PATH);
  char file_arg[LEN_MAX_PATH];
  strcpy (file_arg, argv);
  file.base_name = basename (file_arg);

  if (exists (file_arg))
  {
    strcpy (file.relative_path, file_arg);

    truncate_str (file.relative_path, strlen (file.base_name));
    long unsigned int req_len = multi_strlen (file.relative_path, file.relative_info_path, file.base_name, TRASHINFO_EXT, NULL) + 1;
    if (req_len > sizeof file.info)
    {
      print_msg_error ();
      fprintf (stderr, "Overflow averted -- %s -- func:%s\n", file.relative_path, __func__);
      exit (EXIT_BUF_ERR);
    }
    snprintf (file.info, req_len, "%s%s%s%s", file.relative_path, file.relative_info_path,
             file.base_name, TRASHINFO_EXT);

#ifdef DEBUG
    printf ("restore()/debug: %s\n", file.info);
#endif

    FILE *fp;

    if ((fp = fopen (file.info, "r")) != NULL)
    {
        /* adding strlen (path_line) for the 'Path=' preceding the path.
       * multiplying by 3 for worst case scenario (all chars escaped)
       */
      char line[LEN_MAX_TRASHINFO_LINE];
      if (fgets (line, sizeof line, fp) != NULL)
      {
          /**
           * Not using the "[Trash Info]" line, but reading the file
           * sequentially
           */

        if (strncmp (line, st_trashinfo_spec[TI_HEADER].str, st_trashinfo_spec[TI_HEADER].len) == 0)
        {
        }
        else
        {
          display_dot_trashinfo_error (file.info);
          close_file (fp, file.info, __func__);

          return ERR_TRASHINFO_FORMAT;
        }

          /** adding 5 for the 'Path=' preceding the path. */
        if (fgets (line, sizeof line, fp) != NULL)
        {
          char *path_ptr = strchr (line, '=');
          path_ptr++; /* move past the '=' sign */
          unescape_url (path_ptr, file.dest, LEN_MAX_PATH);
          trim_white_space (file.dest);

          close_file (fp, file.info, __func__);
        }
        else
        {
          display_dot_trashinfo_error (file.info);
          close_file (fp, file.info, __func__);

          return ERR_FGETS;
        }

        /* Check for duplicate filename
         */
        if (exists (file.dest))
        {
          bufchk (st_time_var->suffix_added_dup_exists, LEN_MAX_PATH - strlen (file.dest));
          strcat (file.dest, st_time_var->suffix_added_dup_exists);

          if (verbose)
            printf (_("\
Duplicate filename at destination - appending time string...\n"));
        }

        static char parent_dir[LEN_MAX_PATH];

        strcpy (parent_dir, file.dest);

        truncate_str (parent_dir, strlen (basename (file.dest)));

        if (cli_user_options->want_dry_run == false)
          if (! exists (parent_dir))
            make_dir (parent_dir);

        int rename_res = 0;
        if (cli_user_options->want_dry_run == false)
          rename_res = rename (file_arg, file.dest);

        if (!rename_res)
        {
          printf ("+'%s' -> '%s'\n", file_arg, file.dest);

          int result = 0;
          if (cli_user_options->want_dry_run == false)
            result = remove (file.info);

          if (result != 0)
          {
            print_msg_error ();
            printf (_("while removing .trashinfo file: '%s'\n"),
                    file.info);
          }
          else if (verbose)
            printf ("-%s\n", file.info);
        }
        else
        {
          msg_err_rename (file_arg, file.dest, __func__, __LINE__);
          return rename_res;
        }
      }
      else
      {
        print_msg_error ();
        printf ("(fgets) Able to open '%s' but encountered an unknown error\n",
                file.info);
        close_file (fp, file.info, __func__);
        return ERR_FGETS;
      }

    }
    else
    {
      open_err (file.info, __func__);
      return ERR_OPEN;
    }
  }
  else
  {
    /* This printf statement is on a separate line to leave the translated
     * string below it unchanged */
    printf (" :");
    /* TRANSLATORS:  "%s" refers to a file or directory  */
    printf (_("File not found: '%s'\n"), file_arg);
    msg_return_code (FILE_NOT_FOUND);
    return FILE_NOT_FOUND;
  }

  return 0;
}

/*!
 * Displays a list of files that can be restored, user can select multiple
 * files using a curses-bases interface.
 *
 * If rmw isn't built with ncurses wide-character support enabled, a user
 * may experience this minor
 * @bug <a href="https://github.com/theimpossibleastronaut/rmw/issues/205">In some cases, not all files are displayed when using '-s'</a>
 */
int
restore_select (st_waste *waste_head, st_time *st_time_var, const rmw_options * cli_user_options)
{
  st_waste *waste_curr = waste_head;
  int c = 0;

  do
  {
    st_node *root = NULL;
    int n_choices = 0;

    struct dirent *entry = NULL;
    static DIR *waste_dir = NULL;
    waste_dir = opendir (waste_curr->files);
    if (waste_dir == NULL)
    {
      /* we don't want errno to be changed because it's used in msg_error_open_dir()
       * but afaik, endwin() doesn't change errno */
      msg_err_open_dir (waste_curr->files, __func__, __LINE__);
    }
    /*
     *  Sometimes there is a pause before the next waste folder is read.
     * This message should help fill the time before curses is initialized
     * and the list appears
     */
    printf (_("Reading %s...\n"), waste_curr->files);

    while ((entry = readdir (waste_dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      int req_len = multi_strlen (waste_curr->files, entry->d_name, NULL) + 1;
      char full_path[req_len];
      snprintf (full_path, req_len, "%s%s", waste_curr->files, entry->d_name);
      bufchk (full_path, LEN_MAX_PATH);

      struct stat st;
      if (lstat (full_path, &st))
        msg_err_lstat (full_path, __func__, __LINE__);
      char *hr_size = human_readable_size (st.st_size);

      /*
       * The 2nd argument of new_item() (from the curses library, and used
       * below) holds the description.
       *
       */
      char formatted_hr_size[LEN_MAX_HUMAN_READABLE_SIZE];
      snprintf (formatted_hr_size, sizeof formatted_hr_size, "[%s]", hr_size);

      if (S_ISDIR (st.st_mode))
        strcat (formatted_hr_size, " (D)");
      else if (S_ISLNK (st.st_mode))
        strcat (formatted_hr_size, " (L)");

      comparer int_cmp = strcasecmp;
      root = insert_node(root, int_cmp, entry->d_name, formatted_hr_size);

      n_choices++;
    }

    if (closedir (waste_dir))
      msg_err_close_dir (waste_curr->files, __func__, __LINE__);

    /* Initialize curses */
    initscr ();
    clear ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);

    ITEM **my_items;
    MENU *my_menu;
    /* Initialize items */
    my_items = (ITEM **) calloc (n_choices + 1, sizeof (ITEM *));
    populate_menu (root, my_items, true);

    my_items[n_choices] = (ITEM *)NULL;

    my_menu = new_menu ((ITEM **) my_items);
    menu_opts_off (my_menu, O_ONEVALUE);

    mvprintw (LINES - 7, 0, "== %s ==", waste_curr->files);
    mvprintw (LINES - 6, 0,
          ngettext ("== contains %d file ==", "== contains %d files ==", n_choices), n_choices);

    /* TRANSLATORS: I believe the words between the < and > can be translated
     */
    mvprintw (LINES - 4, 0, _("<CURSOR-RIGHT / CURSOR-LEFT> - switch waste folders"));
    mvprintw (LINES - 3, 0, _("\
<SPACE> - select or unselect an item. / <ENTER> - restore selected items"));

    /* TRANSLATORS: the 'q' can not be translated. rmw can only accept a 'q'
     * for input in this case.
    */
    mvprintw (LINES - 2, 0, _("'q' - quit"));
    post_menu (my_menu);
    refresh ();

    do
    {
      c = getch();
      switch (c)
      {
      case KEY_DOWN:
        menu_driver (my_menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver (my_menu, REQ_UP_ITEM);
        break;
      case KEY_LEFT:
        if (waste_curr->prev_node != NULL)
          waste_curr = waste_curr->prev_node;
        else c = 0;
        break;
      case KEY_RIGHT:
        if (waste_curr->next_node != NULL)
          waste_curr = waste_curr->next_node;
        else c = 0;
        break;
      case KEY_NPAGE:
        menu_driver (my_menu, REQ_SCR_DPAGE);
        break;
      case KEY_PPAGE:
        menu_driver (my_menu, REQ_SCR_UPAGE);
        break;
      case ' ':
        menu_driver (my_menu, REQ_TOGGLE_ITEM);
        break;
      }
    } while (c != KEY_RIGHT && c != KEY_LEFT && c != 10 && c != 'q');

    if (c == 10)
    {
      endwin ();
      printf ("\n");
      ITEM **items;
      items = menu_items (my_menu);

      int i;
      for (i = 0; i < item_count (my_menu); ++i)
      {
        if (item_value (items[i]) == TRUE)
        {
          static char recover_file[PATH_MAX + 1];
          snprintf (recover_file, sizeof (recover_file), "%s%s", waste_curr->files, item_name (items[i]));
          /* waste_curr, not waste_head should always be passed here */
          msg_warn_restore(restore (recover_file, st_time_var, cli_user_options));
        }
      }
    }
    if (c != 10)
      endwin ();

    free_item (my_items[0]);
    free_item (my_items[1]);
    free_menu (my_menu);
    dispose (root);

  }while (c != 'q' && c != 10);

  return 0;
}

/*!
 * Restores files from the mrl
 */
void
undo_last_rmw (st_time *st_time_var, const char *mrl_file, const rmw_options * cli_user_options, char *mrl_contents)
{
    int err_ctr = 0;
    char *line = strtok (mrl_contents, "\n");
    while (line != NULL)
    {
      trim_white_space (line);
      int result = restore (line, st_time_var, cli_user_options);
      line = strtok (NULL, "\n");
      msg_warn_restore (result);
      err_ctr += result;
    }

    free (mrl_contents);

    if (err_ctr == 0)
    {
      int result = 0;
      if (cli_user_options->want_dry_run == false)
        result = remove (mrl_file);
      if (result)
      {
        print_msg_error ();
        printf (_("failed to remove %s\n"), mrl_file);
        perror (__func__);
      }

      return;
    }
  return;
}
#endif /* TEST_LIB */
