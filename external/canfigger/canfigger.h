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

#pragma once

#include <limits.h>

#ifndef CANFIGGER_VERSION
#define CANFIGGER_VERSION "0.1.1999"
#endif

#define __CFG_LEN_MAX_LINE (512 + 1)

typedef struct st_canfigger_node st_canfigger_node;
struct st_canfigger_node
{
  // Contains the string that precedes the '=' sign
  char key[__CFG_LEN_MAX_LINE];

  // Contains the string between the '=' sign and the delimiter
  char value[__CFG_LEN_MAX_LINE];

  // Contains the string following the delimiter
  char attribute[__CFG_LEN_MAX_LINE];

  // A pointer to the next node in the list
  st_canfigger_node *next;
};

// Can be used interchangeably for code readability and developer preference
typedef st_canfigger_node st_canfigger_list;


//
// Opens a config file and returns a memory-allocated linked list
// that must be freed later (see canfiggerfree())
//
// Each node is of type st_canfigger_node.
st_canfigger_list *canfigger_parse_file (const char *file,
                                         const char delimiter);

typedef struct st_canfigger_directory st_canfigger_directory;
struct st_canfigger_directory
{
  const char *home;
  char configroot[PATH_MAX];
  char dataroot[PATH_MAX];
};


//
// Frees the list returned by canfigger_parse_file()
void canfigger_free (st_canfigger_node * node);

const st_canfigger_directory *canfigger_get_directories (void);

unsigned short canfigger_realize_str (char *str, const char *homedir);
