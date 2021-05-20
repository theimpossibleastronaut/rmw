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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#if defined HAVE_NCURSESW_MENU_H
#  include <ncursesw/menu.h>
#elif defined HAVE_NCURSES_MENU_H
#  include <ncurses/menu.h>
#elif defined HAVE_MENU_H
#  include <menu.h>
#else
#  error "SysV-compatible Curses Menu header file required"
#endif

#if defined HAVE_NCURSESW_CURSES_H
#  include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#  include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#  include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#  include <ncurses.h>
#elif defined HAVE_CURSES_H
#  include <curses.h>
#else
#  error "SysV or X/Open-compatible Curses header file required"
#endif

#include "trashinfo_rmw.h"
#include "parse_cli_options.h"

#define CTRLD 4

int
restore (const char *src, st_time *st_time_var, const rmw_options * cli_user_options, st_waste *waste_head);

char *
create_formatted_str (const off_t size, const mode_t mode, const char *dir_entry,
                      const int len, char *formatted_hr_size);

st_node *
add_entry (st_node *node, st_waste *waste_curr_node, const char *dir_entry);

int
restore_select (st_waste *waste_head, st_time *st_time_var, const rmw_options * cli_user_options);

int
undo_last_rmw (st_time *st_time_var, const char *mrl_file, const
  rmw_options * cli_user_options, char *mrl_contents, st_waste
  *waste_head);

#ifdef TEST_LIB
char *waste_parent (const char *src);
#endif
