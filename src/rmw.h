/*!
 * @file rmw.h
 * contains most of the global variables used by rmw
 */
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

/*!
 * PATH_MAX is used to set limits on how long a pathname can be.
 * Some systems may not define PATH_MAX so it's defined here if it's
 * not found in limits.h
 */
#ifndef PATH_MAX /* for portability */
#  define PATH_MAX 256
#endif

/*! The MP macro is used as a shortcut throughout the program. */
#define MP (PATH_MAX + 1)

#ifndef ushort
  #define ushort unsigned short int
#endif

typedef struct rmw_options rmw_options;

/** CLI option switches for rmw. */
struct rmw_options
{
  bool want_restore;
  bool want_purge;
  bool want_empty_trash;
  bool want_orphan_chk;
  bool want_selection_menu;
  bool want_undo;
  ushort force;
  /*! list waste folder option */
  bool list;
  /*! Alternate configuration file given at the command line with -c */
  const char *alt_config;
};

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

typedef struct rmw_target rmw_target;

/** Holds information about a file that was specified for rmw'ing
 */
struct rmw_target
{
  /** Replaced by the filename to be rmw'ed, usually specified on the command line */
  char *main_argv;

  /** The absolute path to the file, stored later in a .trashinfo file */
  char real_path[MP];

  /** The basename of the target file, and used for the basename of it's corresponding
   * .trashinfo file */
  char *base_name;

  /** The destination file name. This may be different if a file of the same name already
   *  exists in the WASTE folder */
  char waste_dest_name[MP];

  /** Is <tt>true</tt> if the file exists in the destination WASTE/files folder,
   * false otherwise. If it's a duplicate, a string based on the current time
   * will be appended to \ref dest_name
   */
  bool is_duplicate;
};

typedef struct st_removed st_removed;

/*!
 * Holds a list of files that rmw will be ReMoving.
 */
struct st_removed{
  char file[MP];
  st_removed *next_node;
};

/** Set when rmw is run with the --verbose option. Enables increased output
 * to stdout */
int verbose;

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
  FIRST_RUN,
  ERR_LSTAT
};

/* function prototypes for rmw.c
 * These are only used in rmw.c but prototyping them here to enable
 * using rmw as a library (which is optional but just for people who
 * want to experiment. */

st_removed*
add_removal (st_removed *removals, const char *file);

void
rmw_option_init (struct rmw_options *x);

void
create_undo_file (st_removed *removals, st_removed *removals_head);

void
dispose_removed (st_removed *node);

void
get_time_string (char *tm_str, const ushort len, const char *format, time_t *time_t_now);

bool
is_time_to_purge (time_t time_t_now);

char*
set_time_now_format (void);

#define SECONDS_IN_A_DAY 86400

#endif
