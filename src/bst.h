/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#include <strings.h>
#include <stdlib.h>
#include "config.h"
#include "utils_rmw.h"

#if defined HAVE_NCURSESW_MENU_H
#include <ncursesw/menu.h>
#elif defined HAVE_NCURSES_MENU_H
#include <ncurses/menu.h>
#elif defined HAVE_MENU_H
#include <menu.h>
#else
#error "SysV-compatible Curses Menu header file required"
#endif

typedef int (*comparer) (const char *, const char *);

/*! Used for @ref restore_select() and contains info about each file
 * in the waste folder
 */
typedef struct st_node st_node;
struct st_node
{
  char *file;

  /*! Holds the human readable size of the file */
  char size_str[LEN_MAX_FORMATTED_HR_SIZE];

  /*! Left node of the binary search tree */
  st_node *left;

  /*! Right node of the binary search tree */
  st_node *right;
};

st_node *insert_node (st_node * root, comparer compare, char *file,
                      char *size_str);

void populate_menu (st_node * node, ITEM ** my_items, bool level_one);

void dispose (st_node * root);
