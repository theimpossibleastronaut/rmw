/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2023  Andy Alt (arch_stanton5995@proton.me)
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
set_time_string(char *tm_str, const size_t len, const char *format,
                time_t time_t_now)
{
  struct tm result;
  if (localtime_r(&time_t_now, &result) == NULL)
  {
    fputs
      ("Error: localtime_r() failed for time_t value beyond 32-bit limit.\n",
       stderr);
    exit(EXIT_FAILURE);
  }
  strftime(tm_str, len, format, &result);
  trim_whitespace(tm_str);

  return;
}

/*!
 * Returns a formatted string based on whether or not
 * a fake year is requested at runtime. If a fake-year is not requested,
 * the returned string will be based on the local-time of the user's system.
 */
static void
set_which_deletion_date(char *format)
{
  char *fake_year = getenv("RMW_FAKE_YEAR");
  bool valid_value = false;
  if (fake_year != NULL)
  {
    valid_value = strcasecmp(fake_year, "true") == 0;
    puts("RMW_FAKE_YEAR:true");
  }
  sn_check(snprintf
           (format, LEN_MAX_DELETION_DATE, "%s",
            valid_value ? "1999-%m-%dT%T" : "%FT%T"), LEN_MAX_DELETION_DATE);

  return;
}


void
init_time_vars(st_time *x)
{
  x->now = time(NULL);
  // x->now = 0x80000000;

  set_which_deletion_date(x->t_fmt);

  set_time_string(x->deletion_date, LEN_MAX_DELETION_DATE, x->t_fmt, x->now);

  set_time_string(x->suffix_added_dup_exists,
                  LEN_MAX_TIME_STR_SUFFIX, "_%H%M%S-%y%m%d", x->now);

  return;
}
