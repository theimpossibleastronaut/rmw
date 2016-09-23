/*
 * strings_rmw.h
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_STRINGS_H
#define _INC_STRINGS_H

#include "rmw.h"

#define BUF_ERR -11

enum {
  COPY,
  CONCAT
};

bool
bufchk (const char *str, unsigned short boundary);

bool
bufchk_string_op (bool mode, char *s1, const char *s2, unsigned short boundary);

int
trim (char s[]);

void
erase_char (char c, char *str);

void
trim_slash (char str[]);

void
truncate_str (char *str, unsigned short pos);

bool
resolve_path (const char *str, char *dest);

void
get_time_string (char *tm_str, unsigned short len, const char *format);

bool
escape_url (const char *str, char *dest, unsigned short len);

bool
unescape_url (const char *str, char *dest, unsigned short len);

#endif
