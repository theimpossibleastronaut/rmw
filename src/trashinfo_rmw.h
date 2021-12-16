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

#pragma once

#include "globals.h"
#include <sys/stat.h>

#include "time_rmw.h"

extern const char trashinfo_ext[];
extern const int len_trashinfo_ext;

extern const int TI_LINE_COUNT;

extern const int LEN_MAX_TRASHINFO_PATH_LINE;

/** Each waste directory is added to a linked list and has the data
 * from this structure associated with it.
 */
typedef struct st_waste st_waste;
struct st_waste
{
  /** The parent directory, e.g. $HOME/.local/share/Trash */
  char *parent;

  /*! The info directory (where .trashinfo files are written) will be appended to the parent directory */
  char *info;
  int len_info;

  /** Appended to the parent directory, where files are moved to when they are rmw'ed
   */
  char *files;
  int len_files;

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
  const char *orig;

  /** The absolute path to the file, stored later in a .trashinfo file */
  char *real_path;

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

extern const char *lit_info;
extern const char *path_key;
extern const char *deletion_date_key;

struct st__trashinfo
{
  const char *str;
  const unsigned short int len;
};


typedef struct
{
  char *value;
  union
  {
    char *path_ptr;
    char *date_str_ptr;
  } f;
} trashinfo_field;

int
create_trashinfo (rmw_target * st_f_props, st_waste * waste_curr,
                  st_time * st_time_var);

char *parse_trashinfo_file (const char *file, const char *req_value);
