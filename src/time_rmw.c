/*
 * time_rmw.c
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


#include "rmw.h"
#include "time_rmw.h"
#include "strings_rmw.h"

/*!
 * Assigns a time string to *tm_str based on the format requested
 */
static void
set_time_string (char *tm_str, const int len, const char *format, time_t time_t_now)
{
  struct tm *time_ptr;
  time_ptr = localtime (&time_t_now);
  strftime (tm_str, len, format, time_ptr);
  trim_white_space (tm_str);
  bufchk (tm_str, len);
}

/*!
 * Returns a formatted string based on whether or not
 * a fake year is requested at runtime. If a fake-year is not requested,
 * the returned string will be based on the local-time of the user's system.
 */
static void
set_which_deletion_date (st_time *st_time_var, const int len)
{
  if  (getenv ("RMWTRASH") == NULL ||
      (getenv ("RMWTRASH") != NULL && strcmp (getenv ("RMWTRASH"), "fake-year") != 0))
    snprintf (st_time_var->t_fmt, len, "%s", "%FT%T");
  else
  {
    printf ("  :test mode: Using fake year\n");
    snprintf (st_time_var->t_fmt, len, "%s", "1999-%m-%dT%T");
  }
  return;
}


void
init_time_vars (st_time *st_time_var)
{
  st_time_var->now = time (NULL);

  set_which_deletion_date (st_time_var, LEN_DELETION_DATE);

  set_time_string (
    st_time_var->deletion_date,
    LEN_DELETION_DATE,
    st_time_var->t_fmt,
    st_time_var->now);

  set_time_string (
    st_time_var->suffix_added_dup_exists,
    LEN_TIME_STR_SUFFIX,
    "_%H%M%S-%y%m%d",
    st_time_var->now);

  return;
}


