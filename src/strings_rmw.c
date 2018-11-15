/*
 * strings_rmw.c
 *
 * This file is part of rmw (https://remove-to-waste.info/)
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

#ifndef INC_RMW_H
#define INC_RMW_H
  #include "rmw.h"
#endif

#include <ctype.h>
#include "strings_rmw.h"
#include "messages_rmw.h"

void
bufchk (const char *str, ushort boundary)
{
  /* STR_PART defines the first n characters of the string to display.
   * This assumes 10 will never exceed a buffer size. In this program,
   * there are no buffers that are <= 10 (that I can think of right now)
   */
  #define STR_PART 10

  int rmw_testbuf = 0;
  if (getenv ("RMW_TESTBUF") != NULL)
  {
    printf ("  :test mode: Using RMW_TESTBUF\n");
    rmw_testbuf = atoi (getenv ("RMW_TESTBUF"));
    if (rmw_testbuf > STR_PART + 1 && rmw_testbuf < boundary)

      boundary = rmw_testbuf;
  }

  static ushort len;
  len = strlen (str);

  if (len < boundary)
    return;

  /* TRANSLATORS:  "buffer" in the following instances refers to the amount
   * of memory allocated for a string  */
  printf (_("  :Error: buffer overrun (segmentation fault) prevented.\n"));
  printf (_("If you think this may be a bug, please report it to the rmw developers.\n"));

  /**
   * This will add a null terminator within the boundary specified by
   * display_len, making it possible to view part of the strings to help
   * with debugging or tracing the error.
   */
  static ushort display_len;
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
    fprintf (stderr, _(" <--> Displaying part of the string that caused the error <-->\n\n"));
    fprintf (stderr, "%s\n\n", temp);
  }

  msg_return_code (EXIT_BUF_ERR);
  exit (EXIT_BUF_ERR);
}

/*
 *
 * trim_white_space: remove trailing blanks, tabs, newlines, carriage returns
 *
 */
void
trim_white_space (char *str)
{
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
    str++;

  /* set pointer to segment preceding NULL terminator */
  str--;

  while (isspace ((unsigned int)*str))
  {
    *str = '\0';
    str--;
  }

  return;
}

/*
 * trim_char();
 *
 * Trim a trailing char if present
 *
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
 * adding the null terminator to chop off part of a string
 * at a given position
 */
void
truncate_str (char *str, ushort pos)
{
  str[strlen (str) - pos] = '\0';
}

/**
 * resolve_path()
 *
 * use realpath() to find the absolute path to a file
 *
 * returns 0 if successful
 */
int
resolve_path (const char *src, char *abs_path)
{
  /*
   * dirname() and basename() alters the src string, so making a copy
   */
  char src_temp_dirname[MP];
  char src_temp_basename[MP];

  bufchk (src, MP);
  snprintf (src_temp_dirname, MP, "%s", src);
  snprintf (src_temp_basename, MP, "%s", src);

  if ((realpath (dirname (src_temp_dirname), abs_path)) != NULL)
  {
    strcat (abs_path, "/");
    strcat (abs_path, basename (src_temp_basename));

#ifdef DEBUG
DEBUG_PREFIX
printf ("abs_path = %s in %s\n", abs_path, __func__);
#endif

    bufchk (abs_path, MP);

    return 0;
  }

  printf (_("Error: realpath() returned an error.\n"));
  return 1;
}
