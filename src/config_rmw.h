/*!
 * @file config_rmw.h
 */
/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
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

#include <sys/stat.h>

/* This is always defined when 'configure' is used */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* This is somewhat of an arbitrary value, used for allocating a string
 * with calloc. When the string is tokenized, each element is validated, so
 * if the there's a problem, that's where the check will fail.
 */
#define CFG_LINE_LEN_MAX MP * 2

/* Not currently used
 * #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
 */

typedef struct config_file_option config_file_option;
struct config_file_option {
        char *option;
        int len;
        int position;
};

void translate_config (void);

st_waste *get_config_data (void);
