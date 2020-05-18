/*
 * rmw.h
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 * Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_RMW_H
#define _INC_RMW_H

#include "config.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <locale.h>
#include "gettext.h"
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#ifdef DEBUG
# define DEBUG_PREFIX printf ("[DEBUG] ");
#endif

/* This is always defined when 'configure' is used */
#ifndef VERSION
  #define VERSION "unversioned"
#endif

/*!
 * PATH_MAX is used to set limits on how long a pathname can be.
 * Some systems may not define PATH_MAX so it's defined here if it's
 * not found in limits.h
 */
#ifndef PATH_MAX /* for portability */
#  define PATH_MAX 256
#endif

/*! The LEN_MAX_PATH macro is used as a shortcut throughout the program. */
#define LEN_MAX_PATH (PATH_MAX + 1)

typedef struct rmw_target rmw_target;

/** Holds information about a file that was specified for rmw'ing
 */
struct rmw_target
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
};

#define RETURN_CODE_OFFSET 10
enum {
  EXIT_FAILED_GETENV = RETURN_CODE_OFFSET,
  NO_WASTE_FOLDER,
  EXIT_BUF_ERR,
  EXIT_MALLOC_ERR,
  MAKE_DIR_SUCCESS,
  MAKE_DIR_FAILURE,
  ERR_OPEN,
  ERR_CLOSE,
  ERR_FGETS,
  ERR_TRASHINFO_FORMAT,
  FILE_NOT_FOUND,
  ERR_LSTAT
};

#endif
