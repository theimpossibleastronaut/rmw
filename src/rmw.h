/*
 * rmw.h
 *
 * This file is part of rmw (https://github.com/theimpossibleastronaut/rmw/wiki)
 *
 * Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
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

#define _XOPEN_SOURCE 600

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

/* This is always defined when 'configure' is used */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* DATA_DIR is relative to $HOME */
#ifndef DATA_DIR
  #ifndef WIN32
    #define DATA_DIR "/.config/rmw"
  #else
    #define DATA_DIR "/rmw"
  #endif
#endif

#define CFG_FILE DATA_DIR"/config"
#define UNDO_FILE DATA_DIR"/lastrmw"
#define PURGE_DAY_FILE DATA_DIR"/lastpurge"

#define DOT_TRASHINFO ".trashinfo"

#ifndef PATH_MAX /* for portability */
#define PATH_MAX 256
#endif

/* MP can be defined before building when running running ./configure
 * ex. '$ CFLAGS=-DMP=n ./configure`
 */
#ifndef MP
/** shorten PATH_MAX to two characters */
#   define MP PATH_MAX + 1
#endif

#ifndef ushort
  #define ushort unsigned short int
#endif

typedef struct st_waste st_waste;

struct st_waste{
  char parent[PATH_MAX + 1];
  char info[PATH_MAX + 1];
  char files[PATH_MAX + 1];
  int dev_num;
  bool removable;
  st_waste *prev_node;
  st_waste *next_node;
};

struct rmw_target
{
  char main_argv[MP];
  char real_path[MP];
  char base_name[MP];
  char dest_name[MP];

  bool is_duplicate;
};

bool verbose;

#define RETURN_CODE_OFFSET 10
enum {
  EXIT_FAILED_GETENV = RETURN_CODE_OFFSET,
  NO_WASTE_FOLDER,
  EXIT_BUF_ERR,
  EXIT_MALLOC_ERR,
  EXIT_OPENDIR_FAILURE,
  DATA_DIR_CREATED,
  MAKE_DIR_SUCCESS,
  MAKE_DIR_FAILURE,
  FORCE_NOT_USED,
  IS_NEW_DAY,
  IS_SAME_DAY,
  ERR_OPEN,
  ERR_CLOSE,
  ERR_FGETS,
  ERR_TRASHINFO_FORMAT,
  FILE_NOT_FOUND,
  ERR_OPEN_CONFIG
};

#endif
