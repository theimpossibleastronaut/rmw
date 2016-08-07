/*
 * rmw.h
 *
 * This file is part of rmw (http://rmw.sf.net)
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
#define VERSION "testing"
#endif

#define DEBUG 0

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* datadir also the user's config directory */
#ifndef DATADIR
#define DATADIR "/.config/rmw/"
#endif

  /* 'config' file will be in DATADIR */
#define CFG DATADIR"config"
#define UNDOFILE DATADIR"lastrmw"
#define PURGE_DAY_FILE DATADIR"lastpurge"

/* Max depth for recursive directory purge function */
#define DEPTH_MAX 128

/* Set a sane limit on maximum number of Waste dirs */
/* Not yet implemented */
#define WASTENUM_MAX 16

// shorten PATH_MAX to two characters
enum
{ MP = PATH_MAX + 1 };

typedef unsigned int uint;
typedef unsigned short ushort;

#define DOT_TRASHINFO ".trashinfo"

struct waste_containers {
  char dir[WASTENUM_MAX][MP];
};

enum {
  parent, info, files
};

uint W_devnum[WASTENUM_MAX];

ushort wasteNum, curWasteNum;

// for buf_check_with_strop()
enum {
  CPY, CAT
};


/* char protected[PROTECT_MAX][MP];
 unsigned int protected_num = 0; */
#define PROTECT_MAX 64

FILE *undo_file_ptr;

char time_now[21];

char time_str_appended[16];

bool verbose;
bool bypass;
bool list;

#endif
