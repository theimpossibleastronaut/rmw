/*
 * primary_funcs.h
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

#ifndef INC_PRIMARY_FUNCS_H
#define INC_PRIMARY_FUNCS_H

#include "rmw.h"
#include "restore_rmw.h"
#include "strings_rmw.h"

int
make_dir (const char *dir);

int
create_trashinfo (struct rmw_target file, struct waste_containers *waste,
                  char *time_now, char *time_str_appended, const short cnum);

bool
file_not_found (const char *filename);

#endif
