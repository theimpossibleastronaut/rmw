/*
 * strings_rmw.h
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
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

bool
buf_check_with_strop (char *s1, const char *s2, bool mode);

bool
buf_check (const char *str, unsigned short boundary);

int
trim (char s[]);

void
erase_char (char c, char *str);

bool
change_HOME (char *t, const char *HOMEDIR);

void
trim_slash (char str[]);

void
truncate_str (char *str, short len);

bool
resolve_path (const char *str, char *dest);

void
get_time_string (char *tm_str, unsigned short len, const char *format);

void
convert_space (char *filename);
