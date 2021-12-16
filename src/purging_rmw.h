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

#ifndef _INC_PURGING_H
#define _INC_PURGING_H

#include <sys/stat.h>
#include <unistd.h>             /* for rmdir() */

#include "time_rmw.h"
#include "config_rmw.h"

bool is_time_to_purge (st_time * st_time_var, const char *data_dir);

int
purge (st_config * st_config_data,
       const rmw_options * cli_user_options,
       st_time * st_time_var, int *orphan_ctr);

short orphan_maint (st_waste * waste_head, st_time * st_time_var,
                    int *orphan_ctr);

int
rmdir_recursive (const char *dirname, short unsigned level, const int force);
#endif
