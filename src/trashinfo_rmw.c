/*! @file trashinfo_rmw.c */
/*
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

#include "trashinfo_rmw.h"
#include "utils_rmw.h"
#include "messages_rmw.h"

struct st__trashinfo st_trashinfo_spec[TI_LINE_COUNT];

const int LEN_MAX_TRASHINFO_LINE = (PATH_MAX * 3 + sizeof ("Path=") + 1);

int
create_trashinfo (rmw_target *st_f_props, st_waste *waste_curr, st_time *st_time_var)
{
  /*
   * there already should have been buffer checking on these 2 when they were
   * initialized
   */
  int req_len = multi_strlen (waste_curr->info, st_f_props->base_name, NULL) + 1;

  /*
   * Make sure there's enough room in file_info_dest
   */
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char final_info_dest[LEN_MAX_PATH];
  snprintf (final_info_dest, req_len, "%s%s", waste_curr->info, st_f_props->base_name);

#ifdef DEBUG
DEBUG_PREFIX
printf ("st_f_props->real_path = %s in %s line %d\n", st_f_props->real_path, __func__, __LINE__);
DEBUG_PREFIX
printf ("st_f_props->base_name = %s in %s line %d\n", st_f_props->base_name, __func__, __LINE__);
#endif

  if (st_f_props->is_duplicate)
  {
    int req_len = multi_strlen (final_info_dest, st_time_var->suffix_added_dup_exists, NULL);
    bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
    strcat (final_info_dest, st_time_var->suffix_added_dup_exists);
  }

  req_len = multi_strlen (final_info_dest, TRASHINFO_EXT, NULL);
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  strcat (final_info_dest, TRASHINFO_EXT);

  FILE *fp = fopen (final_info_dest, "w");
  if (fp != NULL)
  {
    /* Worst case scenario: whole path is escaped, so 3 chars per
     * actual character
     **/
    static char escaped_path[LEN_MAX_PATH * 3];

    if (escape_url (st_f_props->real_path, escaped_path, LEN_MAX_PATH * 3) )
      return close_file (fp, final_info_dest, __func__);

#ifdef DEBUG
DEBUG_PREFIX
printf ("%s\n", st_trashinfo_spec[TI_HEADER].str);
DEBUG_PREFIX
printf ("%s%s\n", st_trashinfo_spec[TI_PATH_LINE].str, escaped_path);
DEBUG_PREFIX
printf ("%s%s\n", st_trashinfo_spec[TI_DATE_LINE].str, st_time_var->deletion_date);
#endif

    fprintf (fp, "%s\n", st_trashinfo_spec[TI_HEADER].str);
    fprintf (fp, "%s%s\n", st_trashinfo_spec[TI_PATH_LINE].str, escaped_path);
    fprintf (fp, "%s%s\n", st_trashinfo_spec[TI_DATE_LINE].str, st_time_var->deletion_date);

    return close_file (fp, final_info_dest, __func__);
  }
  else
  {
    open_err (final_info_dest, __func__);
    return errno;
  }
}

int
validate_trashinfo_file (const char *file, char *line)
{
  FILE *info_file_ptr = fopen (file, "r");
  if (info_file_ptr != NULL)
  {
    int passed = 0;

    while (fgets (line, LEN_MAX_TRASHINFO_LINE, info_file_ptr) != NULL)
    {
      trim_white_space (line);
      if (strncmp (line, st_trashinfo_spec[TI_HEADER].str, st_trashinfo_spec[TI_HEADER].len) == 0 ||
         strncmp (line, st_trashinfo_spec[TI_PATH_LINE].str, st_trashinfo_spec[TI_PATH_LINE].len) == 0 ||
         (strncmp (line, st_trashinfo_spec[TI_DATE_LINE].str, st_trashinfo_spec[TI_DATE_LINE].len) == 0 && strlen (line) == 32))
      {
        passed++;
      }
      else
      {

        break;
      }
    }
    close_file (info_file_ptr, file, __func__);

    if (passed == TI_LINE_COUNT)
      return 1;

    display_dot_trashinfo_error (file);
    return 0;
  }
  else
  {
    open_err (file, __func__);
    return 0;
  }
}

void
init_trashinfo_spec (void)
{
  const char *ti_line[] = {
    "[Trash Info]",
    "Path=",
    "DeletionDate="
  };

  int i = 0;

  while (i < TI_LINE_COUNT)
  {
    st_trashinfo_spec[i].str = ti_line[i];
    st_trashinfo_spec[i].len = strlen (ti_line[i]);
    i++;
  };
}
