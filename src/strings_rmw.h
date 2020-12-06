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

#include <libgen.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

void
bufchk (const char *str, int boundary);

void
bufchk_len (const int len, const int boundary, const char *func, const int line);

int
multi_strlen (const char *argv, ...);

void
trim_white_space (char *str);

void
trim_char (const char c, char *str);

void
truncate_str (char *str, int pos);

int
resolve_path (const char *src, char *abs_path);
