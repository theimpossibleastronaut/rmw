/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#define __USE_XOPEN
#endif
#include <time.h>
#include <dirent.h>

#define LEN_MAX_DELETION_DATE (sizeof "2016-01-01T12:00:00")
#define LEN_MAX_TIME_STR_SUFFIX (sizeof "_000000-000000")
#define LEN_TIME_STR_SUFFIX (LEN_MAX_TIME_STR_SUFFIX - 1)
#define SECONDS_IN_A_DAY (60 * 60 * 24)

extern const int len_date_extension;

/*! Holds variables related to time
 */
typedef struct
{
  char suffix_added_dup_exists[LEN_MAX_TIME_STR_SUFFIX];
  char t_fmt[LEN_MAX_DELETION_DATE];
  char deletion_date[LEN_MAX_DELETION_DATE];
  time_t now;
} st_time;

void init_time_vars (st_time * st_time_var);

time_t get_then_time (const char *raw_deletion_date);

#endif
