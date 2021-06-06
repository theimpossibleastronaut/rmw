/*
This file is part of canfigger<https://github.com/andy5995/canfigger>

Copyright (C) 2021  Andy Alt (andy400-dev@yahoo.com)

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

#ifndef __CANFIGGER_H
#define __CANFIGGER_H

#ifndef CANFIGGER_VERSION
#define CANFIGGER_VERSION "0.1.0"
#endif

typedef struct st_canfigger_node st_canfigger_node;
struct st_canfigger_node
{
  char *key;
  char *value;
  char *attribute;
  st_canfigger_node *next;
};
typedef st_canfigger_node st_canfigger_list;

st_canfigger_list *canfigger_parse_file (const char *file,
                                         const char delimiter);

void canfigger_free (st_canfigger_node * node);

#endif // header guard
