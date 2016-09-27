/*
 * restore_rmw.c
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

#ifndef _RESTORE_RMW_H
#define _RESTORE_RMW_H

#include "rmw.h"
#include "primary_funcs.h"
#include "messages_rmw.h"

short
Restore (int argc, char *argv[], int optind, char *time_str_appended,
          struct waste_containers *waste);

void
restore_select (struct waste_containers *waste, char *time_str_appended);

void
undo_last_rmw (char *time_str_appended, struct waste_containers *waste);

int
getch (void);

int
getche (void);

#endif
