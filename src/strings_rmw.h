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

#include <libgen.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

void
bufchk_len (const int len, const int boundary, const char *func, const int line);

void
sn_check (const int len, const int dest_boundary, const char *func,
            const int line);

int
multi_strlen (const char *argv, ...);

void
trim_whitespace (char *str);

void
trim_char (const char c, char *str);

void
truncate_str (char *str, int pos);
