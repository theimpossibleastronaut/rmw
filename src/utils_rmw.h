/*
 * utils_rmw.h
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2020  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_UTILS_H
#define _INC_UTILS_H

#include <sys/stat.h>
#include <unistd.h>

#include "trashinfo_rmw.h"

#define LEN_MAX_HUMAN_READABLE_SIZE 20

int
make_dir (const char *dir);

bool
exists (const char *filename);

void
dispose_waste (st_waste *node);

char *
human_readable_size (off_t size);

bool
user_verify (void);

bool
is_modified (const char* file, const unsigned long int dev, const unsigned long int inode);

bool
escape_url (const char *str, char *dest, const int len);

bool
unescape_url (const char *str, char *dest, const int len);

#endif
