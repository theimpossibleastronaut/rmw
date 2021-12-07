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

#include "globals.h"
#include "time_rmw.h"
#include "strings_rmw.h"
#include "trashinfo_rmw.h"
#include "messages_rmw.h"

/*!
 * Assigns a time string to *tm_str based on the format requested
 */
static void
set_time_string (char *tm_str, const int len, const char *format, time_t time_t_now)
{
  struct tm result;
  localtime_r (&time_t_now, &result);
  strftime (tm_str, len, format, &result);
  trim_whitespace (tm_str);
}

/*!
 * Returns a formatted string based on whether or not
 * a fake year is requested at runtime. If a fake-year is not requested,
 * the returned string will be based on the local-time of the user's system.
 */
static void
set_which_deletion_date (char *format)
{
  char *fake_year = getenv ("RMW_FAKE_YEAR");
  bool valid_value = false;
  if (fake_year != NULL)
  {
    valid_value = strcasecmp (fake_year, "true") == 0;
    puts ("RMW_FAKE_YEAR:true");
  }
  sn_check (snprintf (format, LEN_MAX_DELETION_DATE, "%s", valid_value ? "1999-%m-%dT%T" : "%FT%T"),
                        LEN_MAX_DELETION_DATE, __func__, __LINE__);

  return;
}


void
init_time_vars (st_time *x)
{
  x->now = time (NULL);

  set_which_deletion_date (x->t_fmt);

  set_time_string (
    x->deletion_date,
    LEN_MAX_DELETION_DATE,
    x->t_fmt,
    x->now);

  set_time_string (
    x->suffix_added_dup_exists,
    LEN_MAX_TIME_STR_SUFFIX,
    "_%H%M%S-%y%m%d",
    x->now);

  return;
}

/*!
 * Get the time a file was rmw'ed by reading the corresponding trashinfo
 * file. Called from purge()
 */
time_t
get_then_time(const char *raw_deletion_date)
{
  struct tm tm_then;
  memset(&tm_then, 0, sizeof(struct tm));
  strptime (raw_deletion_date, "%Y-%m-%dT%H:%M:%S", &tm_then);
  return mktime (&tm_then);
}


