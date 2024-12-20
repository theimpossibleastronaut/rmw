/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2021  Andy Alt (arch_stanton5995@proton.me)
Other authors: https://github.com/theimpossibleastronaut/rmw/blob/master/AUTHORS.md

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>

#include "trashinfo.h"
#include "utils.h"
#include "messages.h"

const size_t LEN_MAX_TRASHINFO_PATH_LINE =
  sizeof "Path=" + LEN_MAX_ESCAPED_PATH - 1;

const struct trashinfo_template trashinfo_template =
  { "[Trash Info]", "Path=", "DeletionDate=" };


int
create_trashinfo(rmw_target *st_f_props, st_waste *waste_curr,
                 st_time *st_time_var)
{
  char *tmp_final_info_dest =
    join_paths(waste_curr->info, st_f_props->base_name);

  size_t req_len =
    strlen(tmp_final_info_dest) + len_trashinfo_ext +
    (st_f_props->is_duplicate ? (LEN_MAX_TIME_STR_SUFFIX - 1) : 0) + 1;
  bufchk_len(req_len, PATH_MAX, __func__, __LINE__);
  if (!(tmp_final_info_dest = realloc(tmp_final_info_dest, req_len)))
    fatal_malloc();
  if (!tmp_final_info_dest)
    exit(ENOMEM);
  char final_info_dest[req_len];

  if (st_f_props->is_duplicate)
    strcat(tmp_final_info_dest, st_time_var->suffix_added_dup_exists);

  strcat(tmp_final_info_dest, trashinfo_ext);
  strcpy(final_info_dest, tmp_final_info_dest);
  free(tmp_final_info_dest);

  FILE *fp = fopen(final_info_dest, "w");
  if (fp != NULL)
  {
    /* Worst case scenario: whole path is escaped, so 3 chars per
     * actual character
     **/
    char *escaped_path = escape_url(st_f_props->real_path);
    if (escaped_path == NULL)
      return close_file(&fp, final_info_dest, __func__);

    char *escaped_path_ptr = escaped_path;
    if (waste_curr->media_root != NULL
        && (waste_curr->dev_num == st_f_props->dev_num))
    {
      escaped_path_ptr = &escaped_path[strlen(waste_curr->media_root)];
      if (*escaped_path_ptr == '/')
        escaped_path_ptr++;
      else
      {
        close_file(&fp, final_info_dest, __func__);
        if (unlink(final_info_dest) != 0)
          perror("unlink");
        print_msg_error();
        fprintf(stderr, "Expected a leading '/' in the pathname '%s'\n",
                escaped_path_ptr);
        free(escaped_path);
        exit(EXIT_FAILURE);
      }
    }

    ssize_t want_size = strlen(trashinfo_template.header) + 1 +
      strlen(trashinfo_template.path_key) +
      strlen(escaped_path_ptr) + 1 +
      strlen(trashinfo_template.deletion_date_key) +
      strlen(st_time_var->deletion_date) + 1;

    int n = fprintf(fp, "%s\n%s%s\n%s%s\n", trashinfo_template.header,
                    trashinfo_template.path_key, escaped_path_ptr,
                    trashinfo_template.deletion_date_key,
                    st_time_var->deletion_date);

    free(escaped_path);

    if (n < 0)
    {
      print_msg_error();
      fprintf(stderr, "fprintf() failed due to an error writing to %s\n",
              final_info_dest);
    }
    else if (n != want_size)
    {
      print_msg_error();
      fprintf(stderr,
              "Expected to write %zu bytes, but wrote %d bytes to %s\n",
              want_size, n, final_info_dest);
    }
    return close_file(&fp, final_info_dest, __func__);
  }
  else
  {
    open_err(final_info_dest, __func__);
    return errno;
  }
}


char *
validate_and_get_value(const char *file, ti_key key)
{
  const uint8_t LEN_DELETION_DATE_KEY_WITH_VALUE = 32;
  struct
  {
    bool header_ok;
    bool path_ok;
    bool date_ok;
  } ti_status = { false, false, false };

  FILE *fp = fopen(file, "r");
  if (fp != NULL)
  {
    char *key_value = NULL;
    uint8_t line_n = 0;

    char fp_line[LEN_MAX_TRASHINFO_PATH_LINE];
    while (fgets(fp_line, LEN_MAX_TRASHINFO_PATH_LINE, fp) != NULL
           && line_n < TI_LINE_MAX)
    {
      trim_whitespace(fp_line);
      char *val_ptr;
      switch (line_n)
      {
      case TI_HEADER:
        ti_status.header_ok =
          (strcmp(fp_line, trashinfo_template.header) == 0);
        break;
      case PATH_KEY:
        ti_status.path_ok =
          (strncmp
           (fp_line, trashinfo_template.path_key,
            strlen(trashinfo_template.path_key)) == 0);
        if (ti_status.path_ok && key == PATH_KEY)
        {
          val_ptr = strchr(fp_line, '=');
          if (val_ptr)
          {
            val_ptr++;          /* move past the '=' sign */
            char *unescaped_path = unescape_url(val_ptr);
            if (!unescaped_path)
              fatal_malloc();
            key_value = unescaped_path;
          }
        }
        break;
      case DELETIONDATE_KEY:
        ti_status.date_ok =
          (strncmp(fp_line, trashinfo_template.deletion_date_key,
                   strlen(trashinfo_template.deletion_date_key)) == 0)
          && strlen(fp_line) == LEN_DELETION_DATE_KEY_WITH_VALUE;
        if (ti_status.date_ok && key == DELETIONDATE_KEY)
        {
          val_ptr = strchr(fp_line, '=');
          if (val_ptr)
          {
            val_ptr++;
            key_value = strdup(val_ptr);
            if (!key_value)
              fatal_malloc();
          }
        }
        break;
      }
      line_n++;
    }
    close_file(&fp, file, __func__);

    if (ti_status.header_ok && ti_status.path_ok && ti_status.date_ok
        && key_value)
      return key_value;

    if (key_value != NULL)
      free(key_value);
    display_dot_trashinfo_error(file);
    return NULL;
  }
  open_err(file, __func__);
  return NULL;
}

//const char *ti_key_to_string(ti_key key)
//{
    //switch (key)
    //{
        //case TI_HEADER: return "TI_HEADER";
        //case PATH_KEY: return "PATH_KEY";
        //case DELETIONDATE_KEY: return "DELETIONDATE_KEY";
        //case TI_LINE_MAX: return "TI_LINE_MAX";
        //default: return "UNKNOWN_KEY";
    //}
//}
