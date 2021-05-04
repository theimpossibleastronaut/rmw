/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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
static
#endif
char *get_waste_parent (const char *src)
{
  char src_copy[strlen (src) + 1];
  strcpy (src_copy, src);
  char *src_dirname = rmw_dirname (src_copy);

  char *one_dir_level = "/..";
  int req_len = multi_strlen (src_dirname, one_dir_level, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char waste_parent_rel_path[req_len];
  sprintf (waste_parent_rel_path, "%s%s", src_dirname, one_dir_level);

  char *waste_parent = realpath (waste_parent_rel_path, NULL);

  if (waste_parent == NULL)
  {
    perror ("::realpath");
    exit (errno);
  }

  return waste_parent;
}


int
restore (const char *src, st_time *st_time_var, const rmw_options * cli_user_options, st_waste *waste_head)
{
  if (exists (src))
  {
    bufchk_len (strlen (src) + 1, LEN_MAX_PATH, __func__, __LINE__);
    char *waste_parent = get_waste_parent (src);

    st_waste *waste_curr = waste_head;
    bool waste_match = false;
    while (waste_curr != NULL)
    {
      if (strcmp (waste_curr->parent, waste_parent) == 0)
      {
        waste_match = true;
        break;
      }
      waste_curr = waste_curr->next_node;
    }

    if (!waste_match)
    {
      print_msg_error ();
      fprintf (stderr, "'%s' is not in a Waste directory.\n", src);
      return 1;
    }

    char src_copy[strlen (src) + 1];
    strcpy (src_copy, src);
    char *src_basename = basename (src_copy);

    char src_tinfo[LEN_MAX_PATH];
    int req_len = multi_strlen (waste_parent, "/info/", src_basename, trashinfo_ext, NULL) + 1;
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    sprintf (src_tinfo, "%s%s%s%s", waste_parent, "/info/",
             src_basename, trashinfo_ext);

    char *_dest = parse_trashinfo_file (src_tinfo, path_key);
    if (_dest == NULL)
      return 1;

    /* If the path in the Path key is relative, determine which waste folder in which the file
     * being restored resides, get the dirname of that waste folder and prepend it
     * to dest (thereby making the entire path absolute for restoration.
     */
    char dest[LEN_MAX_PATH];
    strcpy (dest, _dest);
    if (*_dest != '/')
    {
      char *media_root = rmw_dirname (waste_parent);
      req_len = multi_strlen (media_root, "/", _dest, NULL) + 1;
      bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
      char tmp[LEN_MAX_PATH];
      sprintf (tmp, "%s/%s", media_root, _dest);
      strcpy (dest, tmp);
    }
    free (waste_parent);
    free (_dest);

    /* Check for duplicate filename
     */
    if (exists (dest))
    {
      bufchk_len (strlen (dest) + LEN_MAX_TIME_STR_SUFFIX, LEN_MAX_PATH, __func__, __LINE__);
      strcat (dest, st_time_var->suffix_added_dup_exists);

      if (verbose)
        printf (_("\
Duplicate filename at destination - appending time string...\n"));
    }

    static char parent_dir[LEN_MAX_PATH];

    strcpy (parent_dir, dest);

    truncate_str (parent_dir, strlen (basename (dest)));

    if (cli_user_options->want_dry_run == false)
    {
      if (! exists (parent_dir))
      {
        if (!rmw_mkdir (parent_dir, S_IRWXU))
        {
          if (verbose)
          {
            msg_success_mkdir (parent_dir);
          }
        }
        else
        {
          msg_err_mkdir (waste_curr->files, __func__);
        }
      }
    }

    int rename_res = 0;
    if (cli_user_options->want_dry_run == false)
      rename_res = rename (src, dest);

    if (!rename_res)
    {
      printf ("+'%s' -> '%s'\n", src, dest);

      int result = 0;
      if (cli_user_options->want_dry_run == false)
        result = remove (src_tinfo);

      if (result != 0)
      {
        print_msg_error ();
        printf (_("while removing .trashinfo file: '%s'\n"),
                src_tinfo);
      }
      else if (verbose >= 2)
        printf ("-%s\n", src_tinfo);
    }
    else
    {
      msg_err_rename (src, dest, __func__, __LINE__);
      return rename_res;
    }
  }
  else
  {
    /* This printf statement is on a separate line to leave the translated
     * string below it unchanged */
    printf (" :");
    /* TRANSLATORS:  "%s" refers to a file or directory  */
    printf (_("File not found: '%s'\n"), src);
    msg_return_code (ENOENT);
    return ENOENT;
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
      if (isdotdir (entry->d_name))
        continue;

      int req_len = multi_strlen (waste_curr->files, entry->d_name, NULL) + 1;
      char full_path[req_len];
      sprintf (full_path, "%s%s", waste_curr->files, entry->d_name);
      bufchk_len (strlen (full_path) + 1, LEN_MAX_PATH, __func__, __LINE__);

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
      sprintf (formatted_hr_size, "[%s]", hr_size);
      free (hr_size);

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

    const int start_line_bottom = 7;
    const int min_lines_required = start_line_bottom + 3;
    /* Initialize curses */
    initscr ();
    if (LINES < min_lines_required)
    {
      endwin();
      printf (_("Your terminal only has %d lines. A minimum of %d lines is required.\n"), LINES, min_lines_required);
      exit (EXIT_FAILURE);
    }
    clear ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);

    ITEM **my_items;
    MENU *my_menu;
    /* Initialize items */
    // Why is the '+1' needed here? (rmw segfaults without it)
    my_items = (ITEM **) calloc (n_choices + 1, sizeof (ITEM *));
    chk_malloc (my_items, __func__, __LINE__);
    populate_menu (root, my_items, true);

    my_menu = new_menu ((ITEM **) my_items);
    set_menu_format(my_menu, LINES - start_line_bottom - 1, 1);
    menu_opts_off (my_menu, O_ONEVALUE);

    mvprintw (LINES - start_line_bottom, 0, "== %s ==", waste_curr->files);
    mvprintw (LINES - (start_line_bottom - 1), 0,
          ngettext ("== contains %d file ==", "== contains %d files ==", n_choices), n_choices);

    /* TRANSLATORS: I believe the words between the < and > can be translated
     */
    mvprintw (LINES - (start_line_bottom - 3), 0, _("<CURSOR-RIGHT / CURSOR-LEFT> - switch waste folders"));
    mvprintw (LINES - (start_line_bottom - 4), 0, _("\
<SPACE> - select or unselect an item. / <ENTER> - restore selected items"));

    /* TRANSLATORS: the 'q' can not be translated. rmw can only accept a 'q'
     * for input in this case.
    */
    mvprintw (LINES - (start_line_bottom - 5), 0, _("'q' - quit"));
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
          static char recover_file[LEN_MAX_PATH];
          sprintf (recover_file, "%s%s", waste_curr->files, item_name (items[i]));
          msg_warn_restore(restore (recover_file, st_time_var, cli_user_options, waste_head));
        }
      }
    }
    if (c != 10)
      endwin ();

    unpost_menu (my_menu);
    free_menu (my_menu);

    int i;
    for(i = 0; i < n_choices; ++i)
      free_item(my_items[i]);

    free (my_items);
    dispose (root);

  }while (c != 'q' && c != 10);

  return 0;
}

/*!
 * Restores files from the mrl
 */
int
undo_last_rmw (st_time *st_time_var, const char *mrl_file, const
  rmw_options * cli_user_options, char *mrl_contents, st_waste
  *waste_head)
{
    int err_ctr = 0;
    char *line = strtok (mrl_contents, "\n");
    while (line != NULL)
    {
      trim_white_space (line);
      int result = restore (line, st_time_var, cli_user_options, waste_head);
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

      return result;
    }
  return err_ctr;
}
