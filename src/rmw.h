/*
 * rmw.h
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andy400-dev@yahoo.com)
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
#define VERSION "2016.10.xx.dev"
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
/* This will provide 16 MAX, and 1 extra that will be "NULL" */
#define WASTENUM_MAX 17

/**
 * How many folder can be set for "protection" in rmwrc
 */
#define PROTECT_MAX 32

#define DOT_TRASHINFO ".trashinfo"

#define START_WASTE_COUNTER -1

/** shorten PATH_MAX to two characters */
#define MP PATH_MAX + 1

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
  char real_path[MP];
  char base_name[MP];
  char dest_name[MP];

  bool is_duplicate;
};

bool verbose;

/**
 * Exit Codes
 */

#define EXIT_FAILED_GETENV 2
#define NO_WASTE_FOLDER -1
#define EXIT_BUF_ERR -9

#endif
