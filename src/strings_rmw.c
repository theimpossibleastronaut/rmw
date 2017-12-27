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

  display_len = (boundary > 80) ? 80 : boundary;

  char temp[display_len];

  strncpy (temp, str, display_len);

  truncate_str (temp, 1);

  printf (_(" <--> Displaying part of the string that caused the error <-->\n\n"));
  printf ("%s\n\n", temp);

  return EXIT_BUF_ERR;

     /**
     * This exit() is related to issue #8
     * https://github.com/andy5995/rmw/issues/8
     */
  // exit (-1);
}

/**
 * trim: remove trailing blanks, tabs, newlines
 * Adapted from The ANSI C Programming Language, 2nd Edition (p. 65)
 * Brian W. Kernighan & Dennis M. Ritchie
 */
int
trim (char s[])
{
  static int n;

  for (n = strlen (s) - 1; n >= 0; n--)
  {
    if (s[n] != ' ' && s[n] != '\t' && s[n] != '\n')
    {
      // Add null terminator regardless
      s[n + 1] = '\0';
      break;
    }

    s[n] = '\0';
  }
  return n;
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
  printf ("resolve_path()/debug: %s\n", abs_path);
#endif

    if ((func_error = bufchk (abs_path, MP)))
      return func_error;

    return 0;
  }

  printf (_("Error: realpath() returned an error.\n"));
  return 1;
}
