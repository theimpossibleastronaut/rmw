/*
 * trashinfo_rmw.c
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

#include <sys/stat.h>
#include "utils_rmw.h"
// #include "strings_rmw.h"
#include "messages_rmw.h"

/**
 * is_unreserved()
 * returns 1 if the character c is unreserved,  else returns 0
 */
static bool is_unreserved (char c)
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
static bool escape_url (const char *str, char *dest, ushort len)
{
  static ushort pos_str;
  static ushort pos_dest;
  pos_str = 0;
  pos_dest = 0;

  while (str[pos_str])
  {
    if (is_unreserved (str[pos_str]))
    {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        printf (_("rmw: %s(): buffer too small (got %hu, needed a minimum of %hu)\n"), __func__, len, pos_dest+2);
        return 1;
      }

      dest[pos_dest] = str[pos_str];
      pos_dest += 1;
    }
    else {
      /* Again, check for overflow (3 chars + '\0') */
      if (pos_dest + 4 > len)
      {
        printf (_("rmw: %s(): buffer too small (got %hu, needed a minimum of %hu)\n"), __func__, len, pos_dest+4);
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

int
create_trashinfo (struct rmw_target file, struct waste_containers *waste,
                  char *time_now, char *time_str_appended, const short int cnum)
{
  static char finalInfoDest[PATH_MAX + 1];

  sprintf (finalInfoDest, "%s%s", waste[cnum].info, file.base_name);

  if (file.is_duplicate)
    strcat (finalInfoDest, time_str_appended);

  strcat (finalInfoDest, DOT_TRASHINFO);

  if (bufchk (finalInfoDest, MP))
    return 1;

  /* Worst case scenario: whole path is escaped, so 3 chars per
   * actual character */
  static char escaped_path[MP * 3];

  FILE *fp = fopen (finalInfoDest, "w");

  if (fp != NULL)
  {
    if (escape_url (file.real_path, escaped_path, MP * 3) )
      return 1;

    fprintf (fp, "[Trash Info]\n");
    fprintf (fp, "Path=%s\n", escaped_path);
    fprintf (fp, "DeletionDate=%s", time_now);

    static short close_err;
    close_err = close_file (fp, finalInfoDest, __func__);
    if (close_err)
      return 1;
  }
  else
  {
    open_err (finalInfoDest, __func__);
    return 1;
  }

  return 0;
}
