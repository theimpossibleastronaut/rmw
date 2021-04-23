/*
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#include "globals.h"
#include <sys/stat.h>

#include "time_rmw.h"

#define TRASHINFO_EXT ".trashinfo"

#define TI_LINE_COUNT 3

extern const int LEN_MAX_TRASHINFO_PATH_LINE;

/** Each waste directory is added to a linked list and has the data
 * from this structure associated with it.
 */
typedef struct st_waste st_waste;
struct st_waste
{
  /** The parent directory, e.g. $HOME/.local/share/Trash */
  char parent[LEN_MAX_PATH];

  /*! The info directory (where .trashinfo files are written) will be appended to the parent directory */
  char info[LEN_MAX_PATH];

  /** Appended to the parent directory, where files are moved to when they are rmw'ed
   */
  char files[LEN_MAX_PATH];

  /** The device number of the filesystem on which the file resides. rmw does
   * not copy files from one filesystem to another, but rather only moves them.
   * They must reside on the same filesystem as a WASTE folder specified in
   * the configuration file.
   */
  dev_t dev_num;

  /** set to <tt>true</tt> if the parent directory is on a removable device,
   * <tt>false</tt> otherwise.
   */
  bool removable;

  /*
   * If a waste folder is at the top level, a relative path will be
   * used (per the Freedesktop trash spec). See
   https://github.com/theimpossibleastronaut/rmw/issues/299 for more
   info */
  char *media_root;

  /** Points to the previous WASTE directory in the linked list
   */
  st_waste *prev_node;

  /** Points to the next WASTE directory in the linked list
   */
  st_waste *next_node;
};

/** Holds information about a file that was specified for rmw'ing
 */
typedef struct
{
  /** Replaced by the filename to be rmw'ed, usually specified on the command line */
  const char *main_argv;

  /** The absolute path to the file, stored later in a .trashinfo file */
  char real_path[LEN_MAX_PATH];

  /** The basename of the target file, and used for the basename of it's corresponding
   * .trashinfo file */
  const char *base_name;

  /** The destination file name. This may be different if a file of the same name already
   *  exists in the WASTE folder */
  char waste_dest_name[LEN_MAX_PATH];

  /** Is <tt>true</tt> if the file exists in the destination WASTE/files folder,
   * false otherwise. If it's a duplicate, a string based on the current time
   * will be appended to \ref dest_name
   */
  bool is_duplicate;
} rmw_target;

extern const char *path_key;
extern const char *deletion_date_key;

struct st__trashinfo {
  const char *str;
  int len;
};

extern struct st__trashinfo st_trashinfo_spec[TI_LINE_COUNT];

enum {
  TI_HEADER,
  TI_PATH_LINE,
  TI_DATE_LINE
};

int
create_trashinfo (rmw_target *st_f_props, st_waste *waste_curr, st_time *st_time_var);

char
*parse_trashinfo_file (const char *file, const char* req_value);

void
init_trashinfo_spec (void);

#endif
