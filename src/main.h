/*
 * main.h
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 * Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
 * Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md
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

#ifndef _INC_MAIN_H
#define _INC_MAIN_H

#include <sys/stat.h>

#include "time_rmw.h"
#include "trashinfo_rmw.h"
#include "parse_cli_options.h"

#define STR_ENABLE_TEST "RMWTEST_HOME"

extern const char *mrl_is_empty;

/*!
 * Holds a list of files that rmw will be ReMoving.
 */
typedef struct st_removed st_removed;
struct st_removed {
  char file[LEN_MAX_PATH];
  st_removed *next_node;
};

/* function prototypes for main.c
 * These are only used in main.c but prototyping them here to enable
 * using rmw as a library (which is optional but just for people who
 * want to experiment. */

const char *
get_home_dir (const char *alternate_home_dir);

const char *
get_data_rmw_home_dir (void);

int
remove_to_waste (
  const int argc,
  char* const argv[],
  st_waste *waste_head,
  st_time *st_time_var,
  const char *mrl_file,
  const rmw_options * cli_user_options);

void
list_waste_folders (st_waste *waste_head);

st_removed*
add_removal (st_removed *removals, const char *file);

void
create_undo_file (st_removed *removals_head, const char *mrl);

void
dispose_removed (st_removed *node);

#endif
