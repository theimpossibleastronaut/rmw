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

short
bufchk (const char *str, unsigned short boundary)
{
  unsigned short len = strlen (str);

  if (len < boundary)
    return 0;

  fprintf (stderr, "Error: buffer overrun (segmentation fault) prevented.\n");
  fprintf (stderr, "If you think this may be a bug, please report it to the rmw developers.\n");

  /**
   * This will add a null terminator within the boundary specified by
   * display_len, making it possible to view part of the strings to help
   * with debugging or tracing the error.
   */
  unsigned short display_len = 0;

  display_len = (boundary > 80) ? 80 : boundary;

  char temp[display_len];

  strncpy (temp, str, display_len);

  truncate_str (temp, 1);

  fprintf (stderr, " <--> Displaying part of the string that caused the error <-->\n\n");
  fprintf (stderr, "%s\n\n", temp);

  return EXIT_BUF_ERR;

     /**
     * This exit() is related to issue #8
     * https://github.com/andy5995/rmw/issues/8
     */
  // exit (-1);
}

bool
bufchk_string_op (bool mode, char *s1, const char *s2, const unsigned short boundary)
{
  unsigned short len1, len2;

  len1 = strlen (s1);
  len2 = strlen (s2);

  if (len1 + len2 < boundary && mode == CONCAT)
  {
    strcat (s1, s2);
    return 0;
  }
  else if (len2 < boundary && mode == COPY)
  {
    strcpy (s1, s2);
    return 0;
  }
  else if (mode != COPY && mode != CONCAT)
  {
    fprintf (stderr, "Mode for function: <%s> must be either COPY or CONCAT\n",
      __func__);
    return 1;
  }

  fprintf (stderr, "Error: string length exceeded\n");
  fprintf (stderr,
      "If you think this may be a bug, please report it to the rmw developers.\n");

  /**
   * This will add a null terminator within the boundary specified by
   * display_len, making it possible to view part of the strings to help
   * with debugging or tracing the error.
   */

  unsigned short display_len = 0;

  display_len = (boundary > 80) ? 80 : boundary;

  /**
   *  s2 is a constant, so we'll make a copy, and then truncate the copy
   */
  char temp[display_len];

  strncpy (temp, s2, display_len - 1);

  fprintf (stderr, " <--> Displaying part of the strings that caused the error <-->\n\n");

  if (mode == CONCAT)
  {
    truncate_str (s1, display_len);
    fprintf (stderr, "String1: %s\n\n", s1);
  }

  fprintf (stderr, "String2: %s\n\n", temp);

  return 1;

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

  return;
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
    return;

  str[len] = '\0';

  return;
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
 *
 * use realpath() to find the absolute path to a file
 *
 * returns 0 if successful
 */
int
resolve_path (const char *src, char *abs_path)
{
  short func_error;

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

#if DEBUG == 1
  printf ("resolve_path()/debug: %s\n", abs_path);
#endif

    if ((func_error = bufchk (abs_path, MP)))
      return func_error;

    return 0;
  }

  fprintf (stderr, "Error: realpath() returned an error.\n");
  return 1;
}

void
get_time_string (char *tm_str, unsigned short len, const char *format)
{
  struct tm *time_ptr;
  time_t now = time (NULL);
  time_ptr = localtime (&now);
  strftime (tm_str, len, format, time_ptr);
  bufchk (tm_str, len);
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
      {
        fprintf(stderr, "rmw: %s(): buffer too small (got %hu, needed at least %hu)\n", __FUNCTION__, len, pos_dest+2);
        return 1;
      }

      dest[pos_dest] = str[pos_str];
      pos_dest += 1;
    }
    else {
      /* Again, check for overflow (3 chars + '\0') */
      if (pos_dest + 4 > len)
      {
        fprintf(stderr, "rmw: %s(): buffer too small (got %hu, needed at least %hu)\n", __FUNCTION__, len, pos_dest+4);
        return 1;
      }

      /* A quick explanation to this printf
       * %% - print a '%'
       * 0  - pad with left '0'
       * 2  - width of string should be 2 (pad if it's just 1 char)
       * hh - this is a byte
       * X  - print hexadecimal form with uppercase letters
       */
      sprintf(dest + pos_dest, "%%%02hhX", str[pos_str]);
      pos_dest += 3;
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
      pos_str += 1;
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        fprintf(stderr, "rmw: %s(): buffer too small (got %hu, needed at least %hu)\n", __FUNCTION__, len, pos_dest+2);
        return 1;
      }

      sscanf(str + pos_str, "%2hhx", dest + pos_dest);
      pos_str += 2;
    }
    else {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        fprintf(stderr, "rmw: %s(): buffer too small (got %hu, needed at least %hu)\n", __FUNCTION__, len, pos_dest+2);
        return 1;
      }

      dest[pos_dest] = str[pos_str];
      pos_str += 1;
    }
    pos_dest++;
  }

  dest[pos_dest] = '\0';

  return 0;
}
