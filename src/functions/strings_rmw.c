/*
 * strings_rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andy400-dev@yahoo.com)
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
 * buf_check_with_strop()
 *
 * FIXME: This function needs optimizing.
 *
 * Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 */
bool
buf_check_with_strop (char *s1, const char *s2, bool mode)
{
  unsigned int len1, len2;
  len1 = strlen (s1);
  len2 = strlen (s2);
  if (len2 < MP && mode == CPY)
  {
    strcpy (s1, s2);
    return 0;
  }
  else if (len1 + len2 < MP && mode == CAT)
  {
    strcat (s1, s2);
    return 0;
  }

  else if (mode > CAT || mode < CPY)
  {
    printf
      ("error in function: buf_check_with_strop() - mode must be either CPY (0) or CAT (1)");
    abort ();
  }

  else
  {
    printf ("Potential buffer overflow caught. rmw terminating.\n");

     /**
     * This exit() is related to issue #8
     * https://github.com/andy5995/rmw/issues/8
     */
    exit (-1);

  }

}

bool
buf_check (const char *str, unsigned short boundary)
{
  unsigned short len = strlen (str);

  if (len >= boundary)
  {
    printf ("Potential buffer overrun caught; rmw terminating.\n");

  /**
   * This exit() is related to issue #8
   * https://github.com/andy5995/rmw/issues/8
   */
    exit (1);
  }

  return 0;
}

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
 * Trim a trailing slash if present. Only checks for 1
 */
void
trim_slash (char str[])
{
  int len;
  len = strlen(str) - 1;

  if (str[len] != '/')
    return ;

  str[len] = '\0';

  return ;
}

/**
 * adding the null terminator to chop off part of a string
 * at a given position
 */
void
truncate_str (char *str, unsigned short pos)
{
  str[strlen (str) - pos] = '\0';
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

/**
 * is_unreserved()
 * returns 1 if the character c is unreserved,  else returns 0
 */
bool is_unreserved (char c)
{
  /* According to RFC2396, we must escape any character that's
   * reserved or not available in US-ASCII, for simplification, here's
   * the character that we must accept:
   *
   * - Alphabethics (A-Z and a-z)
   * - Numerics (0-9)
   * - The following charcters: ~ _ - .
   *
   * For purposes of this application we will not convert "/"s, as in
   * this case they correspond to their semantic meaning.
   */
  if ( ('A' <= c && c <= 'Z') ||
       ('a' <= c && c <= 'z') ||
       ('0' <= c && c <= '9') )
    return 1;

  switch (c)
  {
    case '-': case '_': case '~': case '.': case '/':
      return 1;

    default:
      return 0;
  }

}

/**
 * escape_url()
 *
 * Convert str into a URL valid string, escaping when necesary
 * returns 0 on success, 1 on failure
 */
bool
escape_url (const char *str, char *dest, unsigned short len)
{

  unsigned short pos_str = 0;
  unsigned short pos_dest = 0;
  while (str[pos_str])
  {
    if (is_unreserved (str[pos_str]))
    {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
        return 1;

      dest[pos_dest] = str[pos_str];
      pos_dest+=1;
    }
    else {
      /* Again, check for overflow (3 chars + '\0') */
      if (pos_dest + 4 > len)
        return 1;

      /* A quick explanation to this printf
       * %% - print a '%'
       * 0  - pad with left '0'
       * 2  - width of string should be 2 (pad if it's just 1 char)
       * hh - this is a byte
       * X  - print hexadecimal form with uppercase letters
       */
      sprintf(dest + pos_dest, "%%%02hhX", str[pos_str]);
      pos_dest+=3;
    }
    pos_str++;
  }

  dest[pos_dest] = '\0';

  return 0;
}

/**
 * unescape_url()
 *
 * Convert a URL valid string into a regular string, unescaping any '%'s
 * that appear.
 * returns 0 on succes, 1 on failure
 */
bool
unescape_url (const char *str, char *dest, unsigned short len)
{
  unsigned short pos_str = 0;
  unsigned short pos_dest = 0;

  while (str[pos_str])
  {
    if (str[pos_str] == '%')
    {
      /* skip the '%' */
      pos_str+=1;
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
        return 1;

      sscanf(str + pos_str, "%2hhx", dest + pos_dest);
      pos_str+=2;
    }
    else {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
        return 1;

      dest[pos_dest] = str[pos_str];
      pos_str+=1;
    }
    pos_dest++;
  }

  dest[pos_dest] = '\0';

  return 0;
}
