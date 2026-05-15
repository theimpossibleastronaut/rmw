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
 */
char *find_topdir_trash(const char *file_path, const char *uid);
