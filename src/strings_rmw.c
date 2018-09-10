/*
 * strings_rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2017  Andy Alt (andy400-dev@yahoo.com)
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

#include "strings_rmw.h"

short
bufchk (const char *str, ushort boundary)
{
  /* str_part defines the first n characters of the string to display.
   * This assumes 10 will never exceed a buffer size. In this program,
   * there are no buffers that are <= 10 (that I can think of right now)
   */
  const ushort str_part = 10;
  static ushort len;
  len = strlen (str);

  if (len < boundary)
    return 0;

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

  display_len = (boundary > str_part) ? str_part : boundary;

  char temp[display_len];
  strncpy (temp, str, display_len);
  temp[display_len - 1] = '\0';

  /* This was replaced by the line above.
   *
   * I'm not sure why this didn't work to prevent the problem
   * mentioned in https://github.com/andy5995/rmw/issues/156 */
  // truncate_str (temp, 1);

  fprintf (stderr, _(" <--> Displaying part of the string that caused the error <-->\n\n"));
  fprintf (stderr, "%s\n\n", temp);

  return EXIT_BUF_ERR;
}

/**
 * trim: remove trailing blanks, tabs, newlines, carriage returns
 */
void
trim (char *str)
{
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
  {
    str++;
  };

  /* set pointer to segment preceding NULL terminator */
  str--;

  /* /r added to fix a bug. It was failing on files when the lines ended
   * with CRLF */
  while (*str == ' ' || *str == '\t' || *str == '\n' || *str == EOF || *str == '\r')
  {
    *str = '\0';
    str--;
  }

  return;
}

/**
 * Trim a trailing slash if present. Only checks for 1
 */
void
trim_slash (char str[])
{
  static int len;
  len = strlen(str) - 1;

  if (str[len] != '/')
    return;

  str[len] = '\0';

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
  static short func_error;
  func_error = 0;

  /*
   * dirname() and basename() alters the src string, so making a copy
   */
  char src_temp_dirname[MP];
  char src_temp_basename[MP];

  strcpy (src_temp_dirname, src);
  strcpy (src_temp_basename, src);

  if ((realpath (dirname (src_temp_dirname), abs_path)) != NULL)
  {
    strcat (abs_path, "/");
    strcat (abs_path, basename (src_temp_basename));

#ifdef DEBUG
DEBUG_PREFIX
printf ("abs_path = %s in %s\n", abs_path, __func__);
#endif

    if ((func_error = bufchk (abs_path, MP)))
      return func_error;

    return 0;
  }

  printf (_("Error: realpath() returned an error.\n"));
  return 1;
}
