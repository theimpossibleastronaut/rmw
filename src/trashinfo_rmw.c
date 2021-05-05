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

const char trashinfo_ext[] = ".trashinfo";
const int len_trashinfo_ext = (sizeof (trashinfo_ext) - 1); /* Subtract 1 for the terminating NULL */
const int LEN_MAX_TRASHINFO_PATH_LINE = (sizeof ("Path=") - 1) + LEN_MAX_ESCAPED_PATH;

const char *path_key = "Path";
const char *deletion_date_key = "DeletionDate";


int
create_trashinfo (rmw_target *st_f_props, st_waste *waste_curr, st_time *st_time_var)
{
  int req_len = strlen (st_f_props->base_name) + waste_curr->len_info + len_trashinfo_ext + LEN_TIME_STR_SUFFIX + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char final_info_dest[req_len];
  sprintf (final_info_dest, "%s%s", waste_curr->info, st_f_props->base_name);

  if (st_f_props->is_duplicate)
  {
    bufchk_len (strlen (final_info_dest) + LEN_MAX_TIME_STR_SUFFIX, LEN_MAX_PATH, __func__, __LINE__);
    strcat (final_info_dest, st_time_var->suffix_added_dup_exists);
  }

  req_len = strlen (final_info_dest) + len_trashinfo_ext + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  strcat (final_info_dest, trashinfo_ext);

  FILE *fp = fopen (final_info_dest, "w");
  if (fp != NULL)
  {
    /* Worst case scenario: whole path is escaped, so 3 chars per
     * actual character
     **/
    static char escaped_path[LEN_MAX_ESCAPED_PATH];

    if (escape_url (st_f_props->real_path, escaped_path, LEN_MAX_ESCAPED_PATH) )
      return close_file (fp, final_info_dest, __func__);

    char *escaped_path_ptr = escaped_path;
    if (waste_curr->media_root != NULL)
    {
      escaped_path_ptr = &escaped_path[strlen (waste_curr->media_root)];
      if (*escaped_path_ptr == '/')
        escaped_path_ptr++;
      else
      {
        print_msg_error ();
        fprintf (stderr, "Expected a leading '/' in the pathname '%s'\n", escaped_path_ptr);
        exit (EXIT_FAILURE);
      }
    }

    fprintf (fp, "%s\n", st_trashinfo_spec[TI_HEADER].str);
    fprintf (fp, "%s%s\n", st_trashinfo_spec[TI_PATH_LINE].str, escaped_path_ptr);
    fprintf (fp, "%s%s\n", st_trashinfo_spec[TI_DATE_LINE].str, st_time_var->deletion_date);

    return close_file (fp, final_info_dest, __func__);
  }
  else
  {
    open_err (final_info_dest, __func__);
    return errno;
  }
}


/*
 * name: parse_trashinfo_file
 *
 * Checks the integrity of a trashinfo file and returns req_value for
 * either the Path or DeletionDate key
 *
 */
char
*parse_trashinfo_file (const char *file, const char* req_value)
{
  if (strcmp (req_value, path_key) != 0 && strcmp (req_value, deletion_date_key) != 0)
  {
    print_msg_error ();
    fprintf (stderr, "Required arg for %s can be either \"%s\" or \"%s\".", __func__, path_key, deletion_date_key);
    return NULL;
  }

  int line_no = 0;
  FILE *fp = fopen (file, "r");
  if (fp != NULL)
  {
    char *value = malloc (LEN_MAX_ESCAPED_PATH);
    chk_malloc (value, __func__, __LINE__);
    bool res = true;
    char fp_line[LEN_MAX_TRASHINFO_PATH_LINE];
    while (fgets (fp_line, LEN_MAX_TRASHINFO_PATH_LINE, fp) != NULL && res == true)
    {
      trim_white_space (fp_line);

      switch (line_no) {
        case TI_HEADER:
          res = strncmp (fp_line, st_trashinfo_spec[TI_HEADER].str, st_trashinfo_spec[TI_HEADER].len) == 0;
          break;
        case TI_PATH_LINE:
          res = strncmp (fp_line, st_trashinfo_spec[TI_PATH_LINE].str, st_trashinfo_spec[TI_PATH_LINE].len) == 0;
          if (res && strcmp (req_value, path_key) == 0)
          {
            char *path_ptr = strchr (fp_line, '=');
            path_ptr++; /* move past the '=' sign */
            unescape_url (path_ptr, value, LEN_MAX_PATH);
            trim_white_space (value);
          }
          break;
        case TI_DATE_LINE:
          res = strncmp (fp_line, st_trashinfo_spec[TI_DATE_LINE].str, st_trashinfo_spec[TI_DATE_LINE].len) == 0
                && strlen (fp_line) == 32;

          if (res && strcmp (req_value, deletion_date_key) == 0)
          {
            char *date_str_ptr = strchr (fp_line, '=');
            date_str_ptr++;
            strcpy (value, date_str_ptr);
            value = realloc (value, strlen (date_str_ptr) + 1);
            chk_malloc (value, __func__, __LINE__);
          }
          break;
        default:
          res = false;
          break;
        }
        line_no++;
      }
    close_file (fp, file, __func__);

    if (res && line_no == TI_LINE_COUNT)
      return value;

    free (value);
    display_dot_trashinfo_error (file);
    return NULL;
  }
  else
  {
    open_err (file, __func__);
    return NULL;
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
