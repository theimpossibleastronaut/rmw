/*
 * strings_rmw.c
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <ctype.h>
#include "strings_rmw.h"
#include "messages_rmw.h"
#include "utils_rmw.h"

/*!
 * Called by other functions to make sure a string has a value. Any function
 * that calls this should have received a string with a value. If the
 * string is NULL or has a length of 0, the code needs to be reviewed to
 * determine why the calling function received an incorrect string.
 *
 * @param[in] str The string to check
 * @param[in] func The name of the calling function
 * @return void
 */
static void
entry_NULL_check (const char *str, const char *func)
{
  if (str != NULL && strlen (str) != 0)
  return;

  print_msg_error ();
  fprintf (stderr,
           "A NULL string was passed to %s. That should not happen.\n\
Please report this bug to the rmw developers.\n", func);
#ifndef TEST_LIB
  exit (EXIT_FAILURE);
#else
  errno = 1;
  return;
#endif
}


/*!
 * Verify that len doesn't exceed boundary, otherwise exit with an error code.
 * Usually used before concatenating 2 or more strings. Program will exit
 * with an error code if len exceeds boundary. len should already have space
 * for the null terminator when this function is called.
 */
void
bufchk_len (const int len, const int dest_boundary, const char *func,
            const int line)
{
  if (len <= dest_boundary)
    return;

  print_msg_error ();
  msg_err_buffer_overrun (func, line);

#ifdef TEST_LIB
  errno = 1;
  return;
#endif

  exit (EBUF);
}

/*!
 * Get the combined length of multiple strings. Last argument MUST be "NULL".
 *
 * @param[in] argv The first argument
 * @param[in] ... variable argument list, each argument must be a string.
 * @return the combined length of each string
 */
int
multi_strlen (const char *argv, ...)
{
  va_list vlist;
  char *str;
  int len = 0;

  str = (char *) argv;
  va_start (vlist, argv);
  do
  {
    len += strlen (str);
    str = va_arg (vlist, char *);
  }
  while (str != NULL);
  va_end (vlist);
  return len;
}

/*!
 * Removes trailing white space from a string (including newlines, formfeeds,
 * tabs, etc
 * @param[out] str The string to be altered
 * @return void
 */
void
trim_white_space (char *str)
{
  entry_NULL_check (str, __func__);

#ifdef TEST_LIB
  if (errno)
    return;
#endif

  char *pos_0 = str;
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
    str++;

  /* set pointer to segment preceding NULL terminator */
  if (str != pos_0)
    str--;
  else
    return;

  while (isspace (*str))
  {
    *str = '\0';
    if (str != pos_0)
      str--;
    else
      break;
  }

  return;
}

/*!
 * Trim a trailing character of a string
 * @param[in] c The character to erase
 * @param[out] str The string to alter
 * @return void
 */
void
trim_char (const char c, char *str)
{
  trim_white_space (str);
  while (*str != '\0')
    str++;

  str--;

  if (*str == c)
    *str = '\0';

  return;
}

/**
 * Add a null terminator to chop off part of a string
 * at a given position
 * @param[in] pos The position at which to add the \0 character
 * @param[out] str The string to change
 * @return void
 */
void
truncate_str (char *str, int pos)
{
  str[strlen (str) - pos] = '\0';
}


/*
 * name: resolve_path()
 *
 * Gets the absolute path + filename
 *
 * realpath() by itself doesn't give the desired result if basename is
 * a dangling  * symlink; this function is designed to do that.
 *
 */
char
*resolve_path (const char *file, const char *b)
{
  int req_len = strlen (file) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char tmp[req_len];
  strcpy (tmp, file);

  char *orig_dirname = realpath (rmw_dirname (tmp), NULL);
  if (orig_dirname == NULL)
  {
    print_msg_error ();
    perror ("realpath()");
    return NULL;
  }

  req_len = multi_strlen (orig_dirname, "/", b, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char *abspath = malloc (req_len);
  chk_malloc (abspath, __func__, __LINE__);
  sprintf (abspath, "%s/%s", orig_dirname, b);
  free (orig_dirname);
  return abspath;
}
