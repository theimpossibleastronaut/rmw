/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2021  Andy Alt (arch_stanton5995@protonmail.com)
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

#pragma once

#include "canfigger.h"

/*!
 * Holds a list of files that rmw will be ReMoving.
 */
typedef struct st_removed st_removed;
struct st_removed
{
  char file[LEN_MAX_PATH];
  st_removed *next_node;
};

typedef struct st_dir st_dir;
struct st_dir
{
  const char *home;
  char configroot[LEN_MAX_PATH];
  char dataroot[LEN_MAX_PATH];
};

typedef struct st_loc st_loc;
struct st_loc
{
  const char *home_dir;
  const char *config_dir;
  const char *config_file;
  const char *data_dir;
  const char *purge_time_file;
  const char *mrl_file;
  const st_dir *st_directory;
};
