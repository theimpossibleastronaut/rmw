/*! @file trashinfo_rmw.h */
/*
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_TRASHINFO_H
#define _INC_TRASHINFO_H

#include "rmw.h"
#include <sys/stat.h>

#include "time_rmw.h"

#define DOT_TRASHINFO ".trashinfo"

#define LEN_MAX_TRASHINFO_LINE (LEN_MAX_PATH * 3 + 5 + 1)

typedef struct st_waste st_waste;

/** Each waste directory is added to a linked list and has the data
 * from this structure associated with it.
 */
struct st_waste
{
  /** The parent directory, e.g. $HOME/.local/share/Trash */
  char parent[PATH_MAX + 1];

  /*! The info directory (where .trashinfo files are written) will be appended to the parent directory */
  char info[PATH_MAX + 1];

  /** Appended to the parent directory, where files are moved to when they are rmw'ed
   */
  char files[PATH_MAX + 1];

  /** The device number of the filesystem on which the file resides. rmw does
   * not copy files from one filesystem to another, but rather only moves them.
   * They must reside on the same filesystem as a WASTE folder specified in
   * the configuration file.
   */
  unsigned int dev_num;

  /** set to <tt>true</tt> if the parent directory is on a removable device,
   * <tt>false</tt> otherwise.
   */
  bool removable;

  /** Points to the previous WASTE directory in the linked list
   */
  st_waste *prev_node;

  /** Points to the next WASTE directory in the linked list
   */
  st_waste *next_node;
};

int
create_trashinfo (rmw_target *st_f_props, st_waste *waste_curr, st_time *st_time_var);

#endif
