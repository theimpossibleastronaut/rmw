/*
 * time_rmw.h
 *
 * Copyright 2019 Andy <andy400-dev@yahoo.com>
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

#ifndef _INC_TIME_RMW_H
#define _INC_TIME_RMW_H

/* for strptime()
 * this macro doesn't get defined on some systems, such as Centos or Slackware,
 * so gives the the warning:
 * "implicit declaration of function 'strptime'"
 *
 * -andy5995 2019-07-30 */
#ifndef __USE_XOPEN
# define __USE_XOPEN
#endif
#include <time.h>
#include <dirent.h>

typedef struct st_time st_time;

#define LEN_MAX_DELETION_DATE (19 + 1)
#define LEN_MAX_TIME_STR_SUFFIX (14 + 1)
#define SECONDS_IN_A_DAY (60 * 60 * 24)

/*! Holds variables related to time
 */
struct st_time {
  char suffix_added_dup_exists[LEN_MAX_TIME_STR_SUFFIX];
  char t_fmt[LEN_MAX_DELETION_DATE];
  char deletion_date[LEN_MAX_DELETION_DATE];
  time_t now;
};


void
init_time_vars (st_time *st_time_var);

time_t
get_then_time(const char *entry_path);

#endif
