/*
 * strings_rmw.c
 *
 * This file is part of rmw (http://rmw.sf.net)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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

#include "strings_rmw.h"

/**
 * trim: remove trailing blanks, tabs, newlines
 * Adapted from The ANSI C Programming Language, 2nd Edition (p. 65)
 * Brian W. Kernighan & Dennis M. Ritchie
 */
int
trim (char s[])
{
  int n;

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
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
void
erase_char (char c, char *str)
{
  int inc = 0;

  while (str[inc] == c)
    inc++;

  if (!inc)
    return;

  int n = strlen (str);
  int i;

  for (i = 0; i < n - inc; i++)
    str[i] = str[i + inc];

  str[n - inc] = '\0';
}

/**
 * if "$HOME" or "~" is used on configuration file
 * change to the value of "HOMEDIR"
 */

bool
change_HOME (char *t, const char *HOMEDIR)
{
  bool status = 0;
  if (t[0] == '~')
  {
    erase_char ('~', t);
    status = 1;
  }
  else if (strncmp (t, "$HOME", 5) == 0)
  {
    int i;

    for (i = 0; i < 5; i++)
      erase_char (t[0], t);

    status = 1;
  }
  else
    return;

  char temp[MP];
  buf_check_with_strop (temp, HOMEDIR, CPY);
  buf_check_with_strop (temp, t, CAT);
  strcpy (t, temp);

}

/**
 * Trim a trailing slash if present. Only checks for 1
 */
int
trim_slash (char s[])
{
  int n;
  n = strlen (s);

  if (s[n - 1] != '/')
    return n;

  for (n - 1; n >= 0; n--)
    if (s[n] == '/')
    {
      s[n] = '\0';
      return n;
    }
}

/**
 * adding the null terminator to chop off part of a string
 * at a given point
 */
void
truncate_str (char *str, short len)
{
  str[strlen (str) - len] = '\0';
}

/**
 * resolve_path()
 * make sure that realpath gave *dest a path
 * returns 0 on success, 1 on failure
 *
 * I had trouble with the actual realpath() return values
 * See http://cboard.cprogramming.com/c-programming/171131-realpath-resolves-path-but-errno-set-2-a.html
 * for more info.
 */
bool
resolve_path (const char *str, char *dest)
{
  dest[0] = '\0';
  realpath (str, dest);

  if (strlen (dest) != 0)
    return 0;

  fprintf (stderr, "Error: realpath() failed on string %s\n", str);
  return 1;
}

void
get_time_string (char *tm_str, unsigned short len, const char *format)
{
  struct tm *time_ptr;
  time_t now = time (NULL);
  time_ptr = localtime (&now);
  strftime (tm_str, len, format, time_ptr);
  buf_check (tm_str, len);
  trim (tm_str);
}
