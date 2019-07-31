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
#include "trashinfo_rmw.h"
#include "messages_rmw.h"

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

/*!
 * Get the time a file was rmw'ed by reading the corresponding trashinfo
 * file. Called from @ref purge()
 */
time_t
get_then_time(struct dirent *entry, const char *entry_path)
{
  bufchk (entry->d_name, MP);

  char trashinfo_line[LEN_TRASHINFO_LINE_MAX];
  *trashinfo_line = '\0';
  time_t then = 0;

  FILE *info_file_ptr = fopen (entry_path, "r");

  if (info_file_ptr != NULL)
  {
    bool passed = 0;
    /*
    * unused  and unneeded Trash Info line.
    * retrieved but not used.
    * Check to see if it's really a .trashinfo file
    */
    if (fgets (trashinfo_line, sizeof trashinfo_line, info_file_ptr)
        != NULL)
    {
      if (strncmp (trashinfo_line, "[Trash Info]", 12) == 0)
        if (fgets
            (trashinfo_line, sizeof trashinfo_line,
             info_file_ptr) != NULL)
          if (strncmp (trashinfo_line, "Path=", 5) == 0)
            if (fgets
                (trashinfo_line, sizeof trashinfo_line,
                 info_file_ptr) != NULL)
            {
              bufchk (trashinfo_line, 40);
              trim_white_space (trashinfo_line);
              if (strncmp (trashinfo_line, "DeletionDate=", 13) == 0
                  && strlen (trashinfo_line) == 32)
                passed = 1;
            }
    }
    close_file (info_file_ptr, entry_path, __func__);

    if (!passed)
    {
      display_dot_trashinfo_error (entry_path);
      return then;
    }
  }
  else
  {
    open_err (entry_path, __func__);
    return then;
  }

  char *date_str_ptr = strchr (trashinfo_line, '=');
  date_str_ptr++;

  struct tm tm_then;
  memset(&tm_then, 0, sizeof(struct tm));
  strptime (date_str_ptr, "%Y-%m-%dT%H:%M:%S", &tm_then);
  then = mktime (&tm_then);
  return then;
}


