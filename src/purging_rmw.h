/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2022  Andy Alt (arch_stanton5995@protonmail.com)
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

#ifndef _INC_PURGING_H
#define _INC_PURGING_H

#include <sys/stat.h>
#include <unistd.h>             /* for rmdir() */

#include "time_rmw.h"
#include "config_rmw.h"

typedef struct st_counters
{
  unsigned int purge;
  unsigned int dirs_containing_files;
  unsigned int max_depth_reached;
  unsigned int deleted_files;
  unsigned int deleted_dirs;
  off_t bytes_freed;
} st_counters;

bool is_time_to_purge (st_time * st_time_var, const char *file);

int
purge (st_config * st_config_data,
       const rmw_options * cli_user_options,
       st_time * st_time_var, int *orphan_ctr);

short orphan_maint (st_waste * waste_head, st_time * st_time_var,
                    int *orphan_ctr);

int
rmdir_recursive (const char *dirname, short unsigned level, const int force,
                 st_counters * ctr);
#endif
