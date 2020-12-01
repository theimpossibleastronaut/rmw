/*
 * strings_rmw.c
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <ctype.h>
#include "strings_rmw.h"
#include "messages_rmw.h"

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
  if (str == NULL || strlen (str) == 0)
  {
#ifndef TEST_LIB
    print_msg_error ();
    fprintf (stderr,
             "A NULL string was passed to %s. That should not happen.\n\
Please report this bug to the rmw developers.", func);
    exit (EXIT_FAILURE);
#else
    errno = 1;
#endif
  }
}

/*!
 * Verify that *str doesn't exceed boundary, otherwise exit with an error code.
 * Used before putting a string into a fixed length array. The main purpose is
 * to avoid having something like "if string > boundary then..." throughout
 * the program where checking the length of a string is required.
 * @param[in] str The string to check
 * @param[in] boundary maximum length, including space for the NULL terminator
 * @return void
 * @see msg_err_buffer_overrun
 * @see bufchk_len
 */
void
bufchk (const char *str, int boundary)
{
  entry_NULL_check (str, __func__);

  /* STR_PART defines the first n characters of the string to display.
   * This assumes 10 will never exceed a buffer size. In this program,
   * there are no buffers that are <= 10 (that I can think of right now)
   */
#define STR_PART 10

  int len;
  len = strlen (str);

  if (len < boundary)
    return;

#ifdef TEST_LIB
  errno = 1;
  return;
#endif

  print_msg_error ();
  /*
   * For now, using arbitrary arguments instead of changing every call to bufchk
   */
  msg_err_buffer_overrun ("unknown", 0);

  /*
   * This will add a null terminator within the boundary specified by
   * display_len, making it possible to view part of the strings to help
   * with debugging or tracing the error.
   */
  int display_len;
  display_len = 0;

  display_len = (boundary > STR_PART) ? STR_PART : boundary;

  char temp[display_len];
  strncpy (temp, str, display_len);
  temp[display_len - 1] = '\0';

  /* Though we've set STR_PART to 10, we don't really know what's "safe",
   * so only display this if verbosity is on
   */
  if (verbose)
  {
    fprintf (stderr,
             _
             (" <--> Displaying part of the string that caused the error <-->\n\n"));
    fprintf (stderr, "%s\n\n", temp);
  }

  msg_return_code (EXIT_BUF_ERR);
  exit (EXIT_BUF_ERR);
}

/*!
 * Verify that len doesn't exceed boundary, otherwise exit with an error code.
 * Usually used before concatenating 2 or more strings. Program will exit
 * with an error code if len exceeds boundary. len should already have space
 * for the null terminator when this function is called.
 * @param[in] len The string to check
 * @param[in] dest_boundary length needed for destination string
 * @param[in] func The calling function
 * @param[in] line The line number from where the function was called
 * @return void
 * @see msg_err_buffer_overrun
 * @see bufchk
 */
void
bufchk_len (const int len, const int dest_boundary, const char *func,
            const int line)
{
  if (len <= dest_boundary)
    return;

#ifdef TEST_LIB
  errno = 1;
  return;
#endif

  print_msg_error ();
  msg_err_buffer_overrun (func, line);
  exit (EXIT_BUF_ERR);
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

/*!
 * use realpath() to find the absolute path to a file
 * @param[in] src The file or dir that needs resolving
 * @param[out] abs_path The absolute path of src
 * @return 0 if real
 *
 * returns 0 if successful
 */
int
resolve_path (const char *src, char *abs_path)
{
  /*
   * dirname() and basename() alters the src string, so making a copy
   */
  char src_temp_dirname[LEN_MAX_PATH];
  char src_temp_basename[LEN_MAX_PATH];

  bufchk (src, LEN_MAX_PATH);
  snprintf (src_temp_dirname, LEN_MAX_PATH, "%s", src);
  snprintf (src_temp_basename, LEN_MAX_PATH, "%s", src);

  if ((realpath (dirname (src_temp_dirname), abs_path)) != NULL)
  {
    strcat (abs_path, "/");
    strcat (abs_path, basename (src_temp_basename));

#ifdef DEBUG
    DEBUG_PREFIX printf ("abs_path = %s in %s\n", abs_path, __func__);
#endif

    bufchk (abs_path, LEN_MAX_PATH);

    return 0;
  }

  print_msg_error ();
  printf (_("realpath() returned an error.\n"));
  return errno;
}
