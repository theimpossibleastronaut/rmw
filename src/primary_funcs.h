/*
 * primary_funcs.h
 *
 * This file is part of rmw (http://rmw.sf.net)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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
#include "functions/restore_rmw.h"

int
mkinfo (bool dup_filename, char *file_basename, char *cur_file,
        struct waste_containers *waste, char *time_now,
        char *time_str_appended, const short cnum);

void
undo_last_rmw (const char *HOMEDIR, char *time_str_appended);

int getch (void);

int getche (void);

bool
buf_check_with_strop (char *s1, const char *s2, bool mode);

bool
buf_check (const char *str, unsigned short boundary);

void
mkdirErr (const char *dirname, const char *text_string);

void
waste_check (const char *p);

bool
file_not_found (const char *filename);
