/*
 * rmw.h
 *
 * This file is part of rmw (https://rmw.sf.net)
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
#define VERSION "2016.08.02.02a"
#endif

#define DEBUG 0

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* datadir also the user's config directory */
#define DATADIR "/.config/rmw/"

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

char file_bn[MP];		/* file base name */
char cur_file[MP];		/* current file   */

/* rmw info file extension */
/* NOTE: make this non-global */
/* extern const char info_EXT[] = ".trashinfo"; */
#define info_EXT ".trashinfo"


// Delete files from Waste after x number of days
// this default value can be overwritten by adding a
// 'purgeDays=' line to config file
uint purgeDays;

char W_cfg[WASTENUM_MAX][MP];
char W_files[WASTENUM_MAX][MP];
char W_info[WASTENUM_MAX][MP];

uint W_devnum[WASTENUM_MAX];

ushort wasteNum, curWasteNum;

// for buf_check_with_strop()
enum
{
  CPY, CAT
};

/* char protected[PROTECT_MAX][MP];
 unsigned int protected_num = 0; */
#define PROTECT_MAX 64

// for command line options
extern bool list;
extern bool verbose;
extern bool bypass;


FILE *undo_file_ptr;

char HOMEDIR[MP];

char present_time[21];
/* time string to append */
char appended_time[16];

bool verbose;
bool bypass;
bool list;

#endif
