/*
 * primary_funcs.c
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

#include "primary_funcs.h"

/**
 * make_dir()
 *
 * Check for the existence of a dir, and create it if not found.
 * Also creates parent directories.
 */
int
make_dir (const char *dir)
{
  if (file_not_found (dir) == 0)
    return 0;

  char temp_dir[MP];
  strcpy (temp_dir, dir);

  char *tokenPtr;
  tokenPtr = strtok (temp_dir, "/");

  char add_to_path[MP];
  add_to_path[0] = '/';
  add_to_path[1] = '\0';

  bool mk_err = 0;

  while (tokenPtr != NULL)
  {
    if (strlen (add_to_path) > 1)
      strcat (add_to_path, "/");

    strcat (add_to_path, tokenPtr);
    tokenPtr = strtok (NULL, "/");

    if (file_not_found (add_to_path))
    {
      mk_err = (mkdir (add_to_path, S_IRWXU));

      if (!mk_err)
        continue;

      else
        break;
    }
  }

  if (!mk_err)
  {
    fprintf (stdout, "Created directory %s\n", dir);
    return 0;
  }

  fprintf (stderr, "Error: while creating %s\n", add_to_path);
  perror ("make_dir()");
  return errno;
}

int
mkinfo (struct rmw_target file, struct waste_containers *waste, char *time_now,
        char *time_str_appended, const short cnum)
{
  FILE *fp;

  char finalInfoDest[PATH_MAX + 1];

  bufchk_string_op (COPY, finalInfoDest, waste[cnum].info, MP);

  bufchk_string_op (CONCAT, finalInfoDest, file.base_name, MP);

  if (file.is_duplicate)
    bufchk_string_op (CONCAT, finalInfoDest, time_str_appended, MP);

  bufchk_string_op (CONCAT, finalInfoDest, DOT_TRASHINFO, MP);

  /* Worst case scenario: whole path is escaped, so 3 chars per
   * actual character */
  char escaped_path[MP * 3];

  fp = fopen (finalInfoDest, "w");

  if (fp != NULL)
  {
    if (escape_url (file.real_path, escaped_path, MP * 3) )
      return 1;

    fprintf (fp, "[Trash Info]\n");
    fprintf (fp, "Path=%s\n", escaped_path);
    fprintf (fp, "DeletionDate=%s", time_now);

    short close_err = close_file (fp, finalInfoDest, __func__);
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

/**
 * file_not_found()
 * Checks for the existence of *filename
 *
 * returns 1 if not found; 0 if the file exists
 */
bool
file_not_found (const char *filename)
{
  struct stat st;
  // bool regular = 0;
  // regular =

  // printf("%s/t/t%d\n", filename, regular);

  // if (regular == 0 || S_ISLNK (st.st_mode) || S_ISDIR (st.st_mode))
  // if (regular == 0)

  if (lstat (filename, &st) == 0)
    return 0;

  else
    return 1;
}
