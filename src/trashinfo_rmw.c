/*! @file trashinfo_rmw.c */
/*
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2019  Andy Alt (andy400-dev@yahoo.com)
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

#include "trashinfo_rmw.h"
#include "utils_rmw.h"
#include "messages_rmw.h"

/*!
  *
  * According to RFC2396, we must escape any character that's
  * reserved or not available in US-ASCII, for simplification, here's
  * the character that we must accept:
  *
  * - Alphabethics (A-Z and a-z)
  * - Numerics (0-9)
  * - The following charcters: ~ _ - .
  *
  * For purposes of this application we will not convert "/"s, as in
  * this case they correspond to their semantic meaning.
  * @param[in] c the character to check
  * returns true if the character c is unreserved
  * @see escape_url
  * @see unescape_url
 */
static bool is_unreserved (char c)
{
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

/*!
 * Convert str into a URL valid string, escaping when necessary
 * @returns 0 on success, 1 on failure
 * @see is_unreserved
 * @see unescape_url
 */
bool escape_url (const char *str, char *dest, const int len)
{
  int pos_str;
  int pos_dest;
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
        fprintf (stderr, _("rmw: %s(): buffer too small (got %d, needed a minimum of %d)\n"), __func__, len, pos_dest+2);
        return 1;
      }

      dest[pos_dest] = str[pos_str];
      pos_dest += 1;
    }
    else {
      /* Again, check for overflow (3 chars + '\0') */
      if (pos_dest + 4 > len)
      {
        fprintf (stderr, _("rmw: %s(): buffer too small (got %d, needed a minimum of %d)\n"), __func__, len, pos_dest+4);
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

#ifdef DEBUG
DEBUG_PREFIX
printf ("dest = %s in %s\n", dest, __func__);
#endif

  return 0;
}

#ifndef TEST_LIB /* for ticket https://github.com/theimpossibleastronaut/rmw/issues/243 */

int
create_trashinfo (rmw_target *st_f_props, st_waste *waste_curr)
{
  /*
   * there already should have been buffer checking on these 2 when they were
   * initialized
   */
  int req_len = multi_strlen (waste_curr->info, st_f_props->base_name, NULL) + 1;

  /*
   * Make sure there's enough room in file_info_dest
   */
  bufchk_len (req_len, MP, __func__, __LINE__);
  char final_info_dest[MP];
  snprintf (final_info_dest, req_len, "%s%s", waste_curr->info, st_f_props->base_name);

#ifdef DEBUG
DEBUG_PREFIX
printf ("st_f_props->real_path = %s in %s line %d\n", st_f_props->real_path, __func__, __LINE__);
DEBUG_PREFIX
printf ("st_f_props->base_name = %s in %s line %d\n", st_f_props->base_name, __func__, __LINE__);
#endif

  if (st_f_props->is_duplicate)
  {
    extern const char *time_str_appended;
    int req_len = multi_strlen (final_info_dest, time_str_appended, NULL);
    bufchk_len (req_len, MP, __func__, __LINE__);
    strncat (final_info_dest, time_str_appended, strlen (final_info_dest));
  }

  req_len = multi_strlen (final_info_dest, DOT_TRASHINFO, NULL);
  bufchk_len (req_len, MP, __func__, __LINE__);
  strncat (final_info_dest, DOT_TRASHINFO, strlen (final_info_dest));

  FILE *fp = fopen (final_info_dest, "w");
  if (fp != NULL)
  {
    /* Worst case scenario: whole path is escaped, so 3 chars per
     * actual character
     **/
    static char escaped_path[MP * 3];

    if (escape_url (st_f_props->real_path, escaped_path, MP * 3) )
      return close_file (fp, final_info_dest, __func__);

    extern const char *time_now;
#ifdef DEBUG
DEBUG_PREFIX
printf ("[Trash Info]\n");
DEBUG_PREFIX
printf ("Path=%s\n", escaped_path);
DEBUG_PREFIX
printf ("DeletionDate=%s\n", time_now);
#endif

    fprintf (fp, "[Trash Info]\n");
    fprintf (fp, "Path=%s\n", escaped_path);
    fprintf (fp, "DeletionDate=%s\n", time_now);

    return close_file (fp, final_info_dest, __func__);
  }
  else
  {
    open_err (final_info_dest, __func__);
    return ERR_OPEN;
  }
}
#endif
