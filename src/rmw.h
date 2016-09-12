/*
 * rmw.h
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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

#ifndef INC_RMW_H
#define INC_RMW_H

/* Enable support for files over 2G  */
#define _FILE_OFFSET_BITS 64

#define _GNU_SOURCE

#include "config.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/stat.h>

#ifndef VERSION
/* #define VERSION "2016.09.04.01a" */
#define VERSION "devel.testing.unstable"
#endif

#define DEBUG 0

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* DATA_DIR is relative to $HOME */
#ifndef DATA_DIR
#define DATA_DIR "/.config/testrmw"
#endif

#define CFG_FILE DATA_DIR"/config"
#define UNDO_FILE DATA_DIR"/lastrmw"
#define PURGE_DAY_FILE DATA_DIR"/lastpurge"

/* Set a sane limit on maximum number of Waste dirs */
/* Not yet fully implemented */
#define WASTENUM_MAX 16

#define PROTECT_MAX 32

// shorten PATH_MAX to two characters
enum
{ MP = PATH_MAX + 1 };

#define DOT_TRASHINFO ".trashinfo"

struct waste_containers
{
  char parent[MP];
  char info[MP];
  char files[MP];

  int dev_num;
};

struct rmw_target
{
  char main_argv[MP];
  char base_name[MP];
  char dest_name[MP];

  bool is_duplicate;
};

/**
 * WIP:
 *
struct messages
{
  char close[81];
};
*/

enum
{
  CPY, CAT
};

#define NO_WASTE_FOLDER -1

bool verbose;

#endif
