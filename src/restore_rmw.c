/*
 * restore_rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2017  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_RMW_H
#define INC_RMW_H
  #include "rmw.h"
#endif

#include "restore_rmw.h"
#include "utils_rmw.h"
#include "messages_rmw.h"
#include "bst.h"

/**
 * unescape_url()
 *
 * Convert a URL valid string into a regular string, unescaping any '%'s
 * that appear.
 * returns 0 on succes, 1 on failure
 */
static bool
unescape_url (const char *str, char *dest, ushort len)
{
  static ushort pos_str;
  static ushort pos_dest;
  pos_str = 0;
  pos_dest = 0;

  while (str[pos_str])
  {
    if (str[pos_str] == '%')
    {
      /* skip the '%' */
      pos_str += 1;
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        printf (_
                ("rmw: %s(): buffer too small (got %hu, needed a minimum of %hu)\n"),
                __func__, len, pos_dest + 2);
        return 1;
      }

      sscanf (str + pos_str, "%2hhx", dest + pos_dest);
      pos_str += 2;
    }
    else
    {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        printf (_
                ("rmw: %s(): buffer too small (got %hu, needed a minimum of %hu)\n"),
                __func__, len, pos_dest + 2);
        return 1;
      }

      dest[pos_dest] = str[pos_str];
      pos_str += 1;
    }
    pos_dest++;
  }

  dest[pos_dest] = '\0';

  return 0;
}

/**
 * FIXME: This apparently needs re-working too. I'm sure it could be
 * written more efficiently
 */
int
Restore (const char *argv, char *time_str_appended, st_waste *waste_curr)
{
  static struct restore
  {
    char *base_name;
    char relative_path[MP];
    char dest[MP];
    char info[MP];
  } file;

  bufchk (argv, MP);
  char file_arg[MP];
  strcpy (file_arg, argv);
  file.base_name = basename (file_arg);

/**
 * The 2 code blocks below address
 * restoring files with only the basename #14
 */
  if ((strcmp (file.base_name, file_arg) == 0) && !exists (file.base_name))
  {
    /* TRANSLATORS:  "basename" refers to the basename of a file  */
    printf (_("Searching using only the basename...\n"));

    while (waste_curr != NULL)
    {
      static char *possibly_in_path;

      possibly_in_path = waste_curr->files;

      strcat (possibly_in_path, file_arg);

      msg_warn_restore (Restore (possibly_in_path, time_str_appended, waste_curr));
      waste_curr = waste_curr->next_node;
    }

    printf (_("search complete\n"));

    return 0;
  }

  if (exists (file_arg))
  {
    strcpy (file.relative_path, file_arg);

    truncate_str (file.relative_path, strlen (file.base_name));

    sprintf (file.info, "%s%s%s%s", file.relative_path, "../info/",
             file.base_name, DOT_TRASHINFO);

#ifdef DEBUG
    printf ("Restore()/debug: %s\n", file.info);
#endif

    bufchk (file.info, MP);

    FILE *fp;

    if ((fp = fopen (file.info, "r")) != NULL)
    {
        /* adding 5 for the 'Path=' preceding the path.
       * multiplying by 3 for worst case scenario (all chars escaped)
       */
      static char line[MP * 3 + 5];
      if (fgets (line, sizeof (line), fp) != NULL)
      {
          /**
           * Not using the "[Trash Info]" line, but reading the file
           * sequentially
           */

        if (strncmp (line, "[Trash Info]", 12) == 0)
        {
        }
        else
        {
          display_dot_trashinfo_error (file.info);
          close_file (fp, file.info, __func__);

          return ERR_TRASHINFO_FORMAT;
        }

          /** adding 5 for the 'Path=' preceding the path. */
        if (fgets (line, MP * 3 + 5, fp) != NULL)
        {
          static char *tokenPtr;

          tokenPtr = strtok (line, "=");
          tokenPtr = strtok (NULL, "=");

            /**
             * tokenPtr now equals the escaped absolute path from the info file
             */
          unescape_url (tokenPtr, file.dest, MP);
          tokenPtr = NULL;
          trim (file.dest);

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
          bufchk (time_str_appended, MP - strlen (file.dest));
          strcat (file.dest, time_str_appended);

          if (verbose)
            printf (_("\
Duplicate filename at destination - appending time string...\n"));
        }

        static char parent_dir[MP];

        strcpy (parent_dir, file.dest);

        truncate_str (parent_dir, strlen (basename (file.dest)));

        /* FIXME: needs error checking */
        if (!exists (parent_dir))
          make_dir (parent_dir);

        int r_result = rename (file_arg, file.dest);
        if (!r_result)
        {
          printf ("+'%s' -> '%s'\n", file_arg, file.dest);

          if (remove (file.info) != 0)
            printf (_("  :Error: while removing .trashinfo file: '%s'\n"),
                    file.info);
          else if (verbose)
            printf ("-%s\n", file.info);
        }
        else
        {
          /* TRANSLATORS: ignore "rename" */
          printf (_("  :Error: Restore (rename) failed: %s\n"), file.dest);
          return r_result;
        }
      }
      else
      {
         printf ("  :Error: (fgets) Able to open '%s' but encountered an unknown error\n",
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

/*
 * restore_select()
 *
 * Displays files that can be restored, user can select a file by
 * entering the corresponding number
 *
 * FIXME: This function needs to be re-worked
 */
int
restore_select (st_waste *waste_curr, char *time_str_appended)
{
  int c = 0;

  /* Initialize curses */
  initscr ();
  cbreak ();
  noecho ();
  keypad (stdscr, TRUE);

  do
  {
    node *root = NULL;
    comparer int_cmp = strcasecmp;
    ITEM **my_items;
    MENU *my_menu;
    clear ();
    int n_choices = 0;

    struct dirent *entry;
    static DIR *waste_dir;
    waste_dir = opendir (waste_curr->files);
    if (waste_dir == NULL)
    {
      /* we don't want errno to be changed because it's used in msg_error_open_dir()
       * but afaik, endwin() doesn't change errno */
      endwin();
      msg_err_open_dir (waste_curr->files, __func__, __LINE__);
    }
    while ((entry = readdir (waste_dir)) != NULL)
    {
      if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, ".."))
        continue;

      char full_path[MP];
      bufchk (full_path, MP - strlen (entry->d_name));
      sprintf (full_path, "%s%s", waste_curr->files, entry->d_name);

      struct stat st;
      lstat (full_path, &st);
      char *hr_size = human_readable_size (st.st_size);
      /* the 2nd argument of new_item() from the ncurses library is for the
      * description.
      */
      char *desc = (char*)calloc (strlen (hr_size) + 2 + 4 + 1, 1);
      sprintf (desc, "[%s]", human_readable_size (st.st_size));

      if (S_ISDIR (st.st_mode))
        strcat (desc, " (D)");
      else if (S_ISLNK (st.st_mode))
        strcat (desc, " (L)");

      root = insert_node(root, int_cmp, entry->d_name, desc);

      n_choices++;
    }

    if (closedir (waste_dir))
    {
      endwin();
      msg_err_close_dir (waste_curr->files, __func__, __LINE__);
    }

    /* Initialize items */
    my_items = (ITEM **) calloc (n_choices + 1, sizeof (ITEM *));
    populate_menu (root, my_items, true);

    /* add 1 for the NULL terminating the ARRAY */
    n_choices++;
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
      ITEM **items;
      items = menu_items (my_menu);

      int i;
      for (i = 0; i < item_count (my_menu); ++i)
      {
        if (item_value (items[i]) == TRUE)
        {
          static char recover_file[PATH_MAX + 1];
          sprintf (recover_file, "%s%s", waste_curr->files, item_name (items[i]));
          msg_warn_restore(Restore (recover_file, time_str_appended, waste_curr));
        }
      }
    }
    free_item (my_items[0]);
    free_item (my_items[1]);
    free_menu (my_menu);
    dispose (root);

  }while (c != 'q' && c != 10);

  /* endwin was already run if c == 10 */
  if (c != 10)
  {
    endwin ();
  }
  return 0;
}

void
undo_last_rmw (char *time_str_appended, st_waste *waste_curr, const char *HOMEDIR)
{
  FILE *undo_file_ptr;
  static char undo_path[MP];
  static char line[MP];

  sprintf (undo_path, "%s%s", HOMEDIR, UNDO_FILE);
  bufchk (undo_path, MP);

  undo_file_ptr = fopen (undo_path, "r");

  if (undo_file_ptr != NULL)
  {
  }
  else
  {
    open_err (undo_path, __func__);
    return;
  }

  int err_ctr = 0;

  while (fgets (line, MP - 1, undo_file_ptr) != NULL)
  {
    int result = 0;
    trim (line);
    result = Restore (line, time_str_appended, waste_curr);
    msg_warn_restore (result);
    err_ctr += result;
  }

  close_file (undo_file_ptr, undo_path, __func__);

  if (err_ctr == 0)
  {
    if (remove (undo_path))
    {
      printf (_(" :warning: failed to remove %s\n"), undo_path);
      perror (__func__);
    }

    return;
  }

  return;
}
