/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2026  Andy Alt (arch_stanton5995@proton.me)
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

/*
 * Given a file path and a uid string, returns the spec-compliant trash
 * directory path for the volume that file_path resides on:
 *   $topdir/.Trash/$uid   if $topdir/.Trash exists, is not a symlink,
 *                         and has the sticky bit set
 *   $topdir/.Trash-$uid   otherwise
 *
 * Returns NULL if the mount point cannot be determined or on error.
 * The caller must free the returned string.
 *
 * If mount_path_out is non-NULL, on success *mount_path_out is set to a
 * newly-malloc'd copy of the mount path that contains file_path (the
 * caller must free it). On failure *mount_path_out is set to NULL.
 */
char *find_topdir_trash(const char *file_path, const char *uid,
                        char **mount_path_out);


typedef struct st_mount_trash st_mount_trash;

struct st_mount_trash
{
  char *mount_path;             /* NULL for the home-trash node */
  char *trash_dir;              /* $topdir/.Trash{,-}/$uid, or home trash dir */
  char *files_dir;              /* trash_dir/files */
  char *info_dir;               /* trash_dir/info  */
  dev_t dev_num;                /* st_dev of mount_path (or of home-trash volume) */
  bool is_home_trash;
  bool is_ficlone_fs;
  st_mount_trash *next;
};

/*
 * Build a linked list of candidate trash locations:
 *   head: the home-trash node ($XDG_DATA_HOME/Trash semantics)
 *   tail: one node per eligible mounted filesystem (FreeDesktop topdir rules)
 *
 * Mounts whose fs type is in the pseudo/network exclusion list, or that are
 * read-only, are skipped. The mount that shares dev_num with home_dev is
 * skipped to avoid duplicating the home-trash node.
 *
 * No directories are created; nodes record the spec-compliant paths only.
 * Returns NULL if even the home-trash node cannot be allocated.
 * Free with free_mount_trash_list().
 */
st_mount_trash *build_mount_trash_list(const char *uid,
                                       const char *home_trash_dir,
                                       dev_t home_dev);

void free_mount_trash_list(st_mount_trash *head);
