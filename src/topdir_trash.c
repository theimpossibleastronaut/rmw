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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <sys/stat.h>
#include <gio/gunixmounts.h>

#include "topdir_trash.h"
#include "messages.h"
#include "strings_rmw.h"
#include "utils.h"


/*
 * Per FreeDesktop Trash spec section 1.1:
 * If $topdir/.Trash exists, is not a symlink, and has the sticky bit set,
 * return "$topdir/.Trash/$uid". Otherwise return "$topdir/.Trash-$uid".
 * Returns NULL on unexpected lstat failure. Caller must free.
 */
static char *
trash_path_for_topdir(const char *topdir, const char *uid)
{
  char *dot_trash = join_paths(topdir, ".Trash");

  struct stat st;
  if (lstat(dot_trash, &st) == 0)
  {
    if (S_ISLNK(st.st_mode))
    {
      print_msg_warn();
      fprintf(stderr, "%s is a symbolic link; ignoring per trash spec\n",
              dot_trash);
    }
    else if (!(st.st_mode & S_ISVTX))
    {
      print_msg_warn();
      fprintf(stderr, "%s lacks the sticky bit; ignoring per trash spec\n",
              dot_trash);
    }
    else
    {
      char *result = join_paths(dot_trash, uid);
      free(dot_trash);
      return result;
    }
  }
  else if (errno != ENOENT)
  {
    free(dot_trash);
    perror("lstat");
    return NULL;
  }

  free(dot_trash);

  char dot_trash_uid[PATH_MAX];
  sn_check(snprintf(dot_trash_uid, sizeof dot_trash_uid, ".Trash-%s", uid),
           sizeof dot_trash_uid);
  return join_paths(topdir, dot_trash_uid);
}


char *
find_topdir_trash(const char *file_path, const char *uid)
{
  /* g_unix_mount_for/g_unix_mount_get_mount_path/g_unix_mount_free were
   * deprecated in GLib 2.78 in favour of g_unix_mount_entry_* equivalents.
   * Suppress warnings until we can set the project minimum to 2.78. */
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GUnixMountEntry *entry = g_unix_mount_for(file_path, NULL);
  if (!entry)
    return NULL;

  const char *topdir = g_unix_mount_get_mount_path(entry);
  char *result = trash_path_for_topdir(topdir, uid);
  g_unix_mount_free(entry);
  G_GNUC_END_IGNORE_DEPRECATIONS

  return result;
}
