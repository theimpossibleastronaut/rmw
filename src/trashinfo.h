/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2024  Andy Alt (arch_stanton5995@proton.me)
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

/** Each waste directory is added to a linked list and has the data
 * from this structure associated with it.
 */
typedef struct st_waste st_waste;
struct st_waste
{
  long path_max;
  long name_max;
  /** The parent directory, e.g. $HOME/.local/share/Trash */
  char *parent;

  /** The info directory (where .trashinfo files are written) will be appended to the parent directory */
  char *info;

  /** Appended to the parent directory, where files are moved to when they are rmw'ed */
  char *files;

  /** If a waste folder is at the top level, a relative path will be
   * used (per the Freedesktop trash spec). See
   * https://github.com/theimpossibleastronaut/rmw/issues/299 for more
   * info */
  char *media_root;

  /** Points to the previous WASTE directory in the linked list */
  st_waste *prev_node;

  /** Points to the next WASTE directory in the linked list */
  st_waste *next_node;

  /** The device number of the filesystem on which the file resides. rmw does
   * not copy files from one filesystem to another, but rather only moves them.
   * They must reside on the same filesystem as a WASTE folder specified in
   * the configuration file.
   */
  dev_t dev_num;

  int len_info;
  int len_files;

  /** Set to <tt>true</tt> if the parent directory is on a removable device,
   * <tt>false</tt> otherwise.
   */
  bool removable;

  bool is_btrfs;
};


/** Holds information about a file that was specified for rmw'ing
 */
typedef struct
{
  /** The absolute path to the file, stored later in a .trashinfo file */
  char *real_path;

  /** The basename of the target file, and used for the basename of its corresponding
   * .trashinfo file */
  const char *base_name;

  /** The device number of the filesystem on which the file resides */
  dev_t dev_num;

  /** The destination file name. This may be different if a file of the same name already
   * exists in the WASTE folder */
  char waste_dest_name[PATH_MAX];

  /** Is <tt>true</tt> if the file exists in the destination WASTE/files folder,
   * false otherwise. If it's a duplicate, a string based on the current time
   * will be appended to \ref dest_name
   */
  bool is_duplicate;
} rmw_target;

extern const struct trashinfo_template
{
  const char *header;
  const char *path_key;
  const char *deletion_date_key;
} trashinfo_template;


int
create_trashinfo(rmw_target * st_f_props, st_waste * waste_curr,
                 st_time * st_time_var);

char *validate_and_get_value(const char *file, ti_key key);
