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

#ifndef TEST_LIB /* for ticket https://github.com/theimpossibleastronaut/rmw/issues/243 */

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
    int req_len = multi_strlen (final_info_dest, st_time_var->suffix_added_dup_exists, NULL);
    bufchk_len (req_len, MP, __func__, __LINE__);
    strncat (final_info_dest, st_time_var->suffix_added_dup_exists, strlen (final_info_dest));
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

#ifdef DEBUG
DEBUG_PREFIX
printf ("[Trash Info]\n");
DEBUG_PREFIX
printf ("Path=%s\n", escaped_path);
DEBUG_PREFIX
printf ("DeletionDate=%s\n", st_time_var->deletion_date);
#endif

    fprintf (fp, "[Trash Info]\n");
    fprintf (fp, "Path=%s\n", escaped_path);
    fprintf (fp, "DeletionDate=%s\n", st_time_var->deletion_date);

    return close_file (fp, final_info_dest, __func__);
  }
  else
  {
    open_err (final_info_dest, __func__);
    return ERR_OPEN;
  }
}
#endif
