/*
 * purging_rmw.h
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

#ifndef _INC_PURGING_H
#define _INC_PURGING_H

#include <sys/stat.h>
#include <unistd.h>             /* for rmdir() */

#include "time_rmw.h"
#include "config_rmw.h"

#ifndef TEST_LIB
  #define RMDIR_MAX_DEPTH 128
#else
  #define RMDIR_MAX_DEPTH 32
#endif

bool is_time_to_purge (st_time * st_time_var, const char *data_dir);

int
purge (st_config * st_config_data,
       const rmw_options * cli_user_options,
       st_time * st_time_var, int *orphan_ctr);

#ifndef TEST_LIB
short orphan_maint (st_waste * waste_head, st_time * st_time_var,
                    int *orphan_ctr);
#else
int
rmdir_recursive (char *dirname, short unsigned level,
                 const int force);
#endif

#endif
