/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2022  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <ctype.h>

#include "parse_cli_options.h"
#include "config_rmw.h"
#include "restore_rmw.h"
#include "utils_rmw.h"
#include "messages_rmw.h"

const char *mrl_is_empty = "[Empty]\n";

#define MENU_KEY_ENTER '\n'
#define MENU_KEY_ESC 27

static char *
get_waste_parent (const char *src)
{
  char src_copy[strlen (src) + 1];
  strcpy (src_copy, src);
  char *src_dirname = rmw_dirname (src_copy);

  char *one_dir_level = "/..";
  char *waste_parent_rel_path = join_paths (src_dirname, one_dir_level, NULL);
  char *waste_parent = realpath (waste_parent_rel_path, NULL);
  free (waste_parent_rel_path);

  if (waste_parent == NULL)
  {
    perror ("::realpath");
    exit (errno);
  }

  return waste_parent;
}


int
restore (const char *src, st_time * st_time_var,
         const rmw_options * cli_user_options, st_waste * waste_head)
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
    if (isdotdir (src_basename))
    {
      printf ("refusing to process '.' or '..' directory: skipping '%s'",
              src_basename);
      return 1;
    }

    char src_tinfo[LEN_MAX_PATH];
    char *tmp_str = join_paths (waste_parent, lit_info, src_basename, NULL);
    int r =
      snprintf (src_tinfo, LEN_MAX_PATH, "%s%s", tmp_str, trashinfo_ext);
    free (tmp_str);
    tmp_str = NULL;
    sn_check (r, LEN_MAX_PATH, __func__, __LINE__);

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
      char *_tmp_str = join_paths (media_root, _dest, NULL);
      strcpy (dest, _tmp_str);
      free (_tmp_str);
    }
    free (waste_parent);
    free (_dest);

    /* Check for duplicate filename
     */
    if (exists (dest))
    {
      bufchk_len (strlen (dest) + LEN_MAX_TIME_STR_SUFFIX, LEN_MAX_PATH,
                  __func__, __LINE__);
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
      if (!exists (parent_dir))
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
        printf (_("while removing .trashinfo file: '%s'\n"), src_tinfo);
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
    msg_warn_file_not_found (src);
    msg_return_code (ENOENT);
    return ENOENT;
  }

  return 0;
}


#if !defined DISABLE_CURSES
// creates a string containing the human readable size of the file and a
// 'D'(directory) or 'L'(link) (For regular files, only the size is displayed).
//
// example: '[4096 Kib] (D)'
//
static char *
create_file_details_str (const off_t size, const mode_t mode)
{
  char hr_size[LEN_MAX_HUMAN_READABLE_SIZE];
  make_size_human_readable (size, hr_size);

  char *file_details = malloc (LEN_MAX_FILE_DETAILS);
  chk_malloc (file_details, __func__, __LINE__);
  sn_check (snprintf
            (file_details, LEN_MAX_FILE_DETAILS, "[%s]", hr_size),
            LEN_MAX_FILE_DETAILS, __func__, __LINE__);

  if (S_ISDIR (mode))
    strcat (file_details, " (D)");
  else if (S_ISLNK (mode))
    strcat (file_details, " (L)");

  return file_details;
}


// A binary search based function to find the position
// where item should be inserted in a[low..high]
// Adapted from https://www.geeksforgeeks.org/c-program-for-binary-insertion-sort/
static int
binary_search (ITEM ** a, ITEM * item, int low, int high)
{
  if (high <= low)
    return (strcasecmp (item_name (item), item_name (a[low])) >
            0) ? (low + 1) : low;

  int mid = (low + high) / 2;

  int r = strcasecmp (item_name (item), item_name (a[mid]));
  if (r == 0)
    return mid + 1;

  if (r > 0)
    return binary_search (a, item, mid + 1, high);
  return binary_search (a, item, low, mid - 1);
}

static void
insertion_sort (ITEM ** a, const int n)
{
  ITEM *selected;
  int i, loc, j;

  for (i = 1; i < n; ++i)
  {
    j = i - 1;
    selected = a[i];

    // find location where selected should be inserted
    loc = binary_search (a, selected, 0, j);

    // Move all elements after location to create space
    while (j >= loc)
    {
      a[j + 1] = a[j];
      j--;
    }
    a[j + 1] = selected;
  }
  return;
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
restore_select (st_waste * waste_head, st_time * st_time_var,
                const rmw_options * cli_user_options)
{
  st_waste *waste_curr = waste_head;
  const int start_line_bottom = 7;
  const int min_lines_required = start_line_bottom + 3;
  int c = 0;

  do
  {
    int n_choices = 0;

    DIR *waste_dir = opendir (waste_curr->files);
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
    struct dirent *entry = NULL;
    // First, get the number of directory entries...
    while ((entry = readdir (waste_dir)) != NULL)
    {
      if (isdotdir (entry->d_name))
        continue;

      n_choices++;
    }

    // Now that we have the number of directory entries, we can
    // allocate memory for all the choices (using n_choices)
    // https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
    //
    // Why is the '+1' needed here? (rmw segfaults without it)
    ITEM **my_items = (ITEM **) calloc (n_choices + 1, sizeof (ITEM *));
    chk_malloc (my_items, __func__, __LINE__);

    rewinddir (waste_dir);
    n_choices = 0;

    while ((entry = readdir (waste_dir)) != NULL)
    {
      if (isdotdir (entry->d_name))
        continue;

      // dir_entry_list = add_entry (dir_entry_list, waste_curr, entry->d_name);
      char *tmp_path = join_paths (waste_curr->files, entry->d_name, NULL);

      // this is used to get the size and mode of the file
      struct stat st;
      if (lstat (tmp_path, &st))
        msg_err_lstat (tmp_path, __func__, __LINE__);
      free (tmp_path);
      char *m_dir_entry = malloc (strlen (entry->d_name) + 1);
      chk_malloc (m_dir_entry, __func__, __LINE__);
      sn_check (snprintf (m_dir_entry, LEN_MAX_PATH, "%s", entry->d_name),
                LEN_MAX_PATH, __func__, __LINE__);
      char *file_details = create_file_details_str (st.st_size, st.st_mode);

      my_items[n_choices] = new_item (m_dir_entry, file_details);
      if (my_items[n_choices] == NULL)
      {
        endwin ();
        perror (m_dir_entry);
        perror ("new_item");
        if (closedir (waste_dir))
          perror ("closedir");
        free (my_items);
        dispose_waste (waste_head);
        exit (EXIT_FAILURE);
      }

      n_choices++;
    }

    if (closedir (waste_dir))
      msg_err_close_dir (waste_curr->files, __func__, __LINE__);

    insertion_sort (my_items, n_choices);
    /* Initialize curses */
    initscr ();
    if (LINES < min_lines_required)
    {
      endwin ();
      printf (_
              ("Your terminal only has %d lines. A minimum of %d lines is required.\n"),
              LINES, min_lines_required);
      dispose_waste (waste_head);
      exit (EXIT_FAILURE);
    }

    clear ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);
    MENU *my_menu = new_menu ((ITEM **) my_items);
    set_menu_format (my_menu, LINES - start_line_bottom - 1, 1);
    menu_opts_off (my_menu, O_ONEVALUE);

    mvprintw (LINES - start_line_bottom, 0, "== %s ==", waste_curr->files);
    mvprintw (LINES - (start_line_bottom - 1), 0,
              ngettext ("== contains %d file ==", "== contains %d files ==",
                        n_choices), n_choices);

    /* TRANSLATORS: I believe the words between the < and > can be translated
     */
    mvprintw (LINES - (start_line_bottom - 3), 0,
              _("<CURSOR-RIGHT / CURSOR-LEFT> - switch waste folders"));
    mvprintw (LINES - (start_line_bottom - 4), 0, _("\
<SPACE> - select or unselect an item. / <ENTER> - restore selected items"));
    mvprintw (LINES - (start_line_bottom - 5), 0, _("<ESC> - quit"));
    post_menu (my_menu);
    refresh ();

    do
    {
      c = getch ();
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
        else
          c = 0;
        break;
      case KEY_RIGHT:
        if (waste_curr->next_node != NULL)
          waste_curr = waste_curr->next_node;
        else
          c = 0;
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
    }
    while (c != KEY_RIGHT && c != KEY_LEFT && c != MENU_KEY_ENTER
           && c != MENU_KEY_ESC && c != 'q');
    // using 'q' to exit will be deprecated some day

    if (c == MENU_KEY_ENTER)
    {
      endwin ();
      putchar ('\n');
      ITEM **items;
      items = menu_items (my_menu);

      int i;
      for (i = 0; i < item_count (my_menu); ++i)
      {
        if (item_value (items[i]) == TRUE)
        {
          char *recover_file =
            join_paths (waste_curr->files, item_name (items[i]), NULL);
          msg_warn_restore (restore
                            (recover_file, st_time_var, cli_user_options,
                             waste_head));
          free (recover_file);
        }
      }
    }
    if (c != MENU_KEY_ENTER)
      endwin ();

    // Some demo menu code at
    // https://github.com/ThomasDickey/ncurses-snapshots/blob/master/test/demo_menus.c
    //
    // The menu must be unposted and freed before freeing items, otherwise valgrind reports
    // substantial memory leak
    unpost_menu (my_menu);
    free_menu (my_menu);

    int i;
    for (i = 0; i < n_choices; i++)
    {
      free ((void *) item_name (my_items[i]));
      free ((void *) item_description (my_items[i]));
      free_item (my_items[i]);
    }

    free (my_items);

  }
  while (c != MENU_KEY_ESC && c != 'q' && c != MENU_KEY_ENTER);

  return 0;
}
#endif

/*!
 * Restores files from the mrl
 */
int
undo_last_rmw (st_time * st_time_var, const char *mrl_file, const
               rmw_options * cli_user_options, char *mrl_contents, st_waste
               * waste_head)
{
  int err_ctr = 0;
  char contents_copy[strlen (mrl_contents) + 1];
  strcpy (contents_copy, mrl_contents);
  char *line = strtok (contents_copy, "\n");
  while (line != NULL)
  {
    trim_whitespace (line);
    int result = restore (line, st_time_var, cli_user_options, waste_head);
    line = strtok (NULL, "\n");
    msg_warn_restore (result);
    err_ctr += result;
  }

  if (err_ctr == 0)
  {
    if (cli_user_options->want_dry_run == false)
    {
      FILE *fd = fopen (mrl_file, "w");
      if (fd != NULL)
      {
        fprintf (fd, "%s", mrl_is_empty);
        close_file (fd, mrl_file, __func__);
      }
      else
        open_err (mrl_file, __func__);
    }
  }
  return err_ctr;
}

///////////////////////////////////////////////////////////////////////
#ifdef TEST_LIB

#include "test.h"

#define BUF_SIZE 80

#if !defined DISABLE_CURSES
static void
test_create_file_details_str (void)
{
  struct expected
  {
    char *name;
    off_t size;
    mode_t mode;
    char *out;
  } const data[] = {
    {"foo", 1204, S_IFDIR, "[1.1 KiB] (D)"},
    {"bar", 10000, S_IFLNK, "[9.7 KiB] (L)"},
    {"Hello", 28000000000000, S_IFREG, "[25.4 TiB]"},
    {"World On A String", 4096, S_IFDIR, "[4.0 KiB] (D)"},
  };

  int data_size = ARRAY_SIZE (data);
  int i = 0;
  while (i < data_size)
  {
    //char file_details[LEN_MAX_FILE_DETAILS];
    //*file_details = '\0';
    char *file_details = create_file_details_str (data[i].size, data[i].mode);
    fprintf (stderr, "file_details: %s\nExpected: %s\n\n", file_details,
             data[i].out);
    assert (strcmp (file_details, data[i].out) == 0);
    free (file_details);
    i++;
  }

  assert (i == data_size);

  return;
}
#endif

int
main ()
{
#if !defined DISABLE_CURSES
  test_create_file_details_str ();
#endif
  return 0;
}
#endif
