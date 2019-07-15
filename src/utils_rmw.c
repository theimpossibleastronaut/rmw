/*!
 * @file utils_rmw.c
 */
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

#ifndef INC_RMW_H
#define INC_RMW_H
  #include "rmw.h"
#endif

#include "utils_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

/**
 * make_dir()
 *
 * Check for the existence of a dir, and create it if not found.
 * Also creates parent directories.
 */
int
make_dir (const char *dir)
{
  if (exists (dir))
    return 0;

  char temp_dir[MP];
  bufchk (dir, MP);
  strcpy (temp_dir, dir);

  char *tokenPtr;
  tokenPtr = strtok (temp_dir, "/");

  char add_to_path[MP];
  if (dir[0] == '/')
    add_to_path[0] = '/';

  add_to_path[1] = '\0';

  bool mk_err = 0;

  while (tokenPtr != NULL)
  {
    if (strlen (add_to_path) > 1 || *add_to_path == '.')
      snprintf (add_to_path + strlen (add_to_path), MP - strlen (add_to_path), "/");

    bufchk (tokenPtr, MP - strlen (add_to_path));
    snprintf (add_to_path + strlen (add_to_path), MP - strlen (add_to_path), "%s", tokenPtr);
    tokenPtr = strtok (NULL, "/");

    if (!exists (add_to_path))
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
    printf (_("Created directory %s\n"), dir);
    return MAKE_DIR_SUCCESS;
  }

  print_msg_error ();
  printf (_("while creating %s\n"), add_to_path);
  perror ("make_dir()");
  return MAKE_DIR_FAILURE;
}

/*!
 * Determine whether or not a file or directory exists.
 * @param[in] filename
 * @return true if file exists, false otherwise
 */
int exists (const char *filename)
{
  FILE *fp = fopen (filename, "r");
  if (fp != NULL)
  {
    close_file (fp, filename, __func__);
    return true;
  }

  return false;
}

void
dispose_waste (st_waste *node)
{
  if (node != NULL)
  {
    dispose_waste (node->next_node);
    node = NULL;
    free (node);
  }
}

char *
human_readable_size (off_t size)
{
  /* "xxxx.y GiB" - 10 chars + '\0' */
  static char buffer[12];

  /* Store only the first letter; we add "iB" later during snprintf(). */
  const char prefix[] = { 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
  short power = -1;

  static short remainder;
  remainder = 0;

  while (size >= 1024)
  {
    remainder = size % 1024;
    size /= 1024;

    ++power;
  }

  if (power >= 0)
    snprintf (buffer, sizeof (buffer), "%ld.%d %ciB", (long) size,
              (remainder * 10) / 1024, prefix[power]);
  else
    snprintf (buffer, sizeof (buffer), "%ld B", (long) size);

  return buffer;
}

/*!
 * Prompt user for confirmation.
 * @return true if 'y' or 'Y' was entered, false otherwise"
 */
bool
user_verify (void)
{
  fputs(_("Continue? (y/n): "), stdout);
  int answer = getchar();
  bool want_continue = (answer == (int)'Y') || (answer == (int)'y');
  int char_count = 0;

  /* Check if there's any more chars */
  while (answer != '\n' && answer != EOF)
  {
    char_count++;
    if (char_count>1024) {
      fputs("Too many chars in stdin!!\n", stderr);
      exit(EXIT_FAILURE);
    }
    answer = getchar();
  }

  return (want_continue && (char_count <= 1));
}

/* used to prevent a TOCTOU race condtion */
bool
is_modified (const char* file, const int dev, const int inode)
{
  struct stat st;
  if (lstat (file, &st))
    msg_err_lstat (__func__, __LINE__);

  if (dev == st.st_dev && inode == st.st_ino)
    return false;

  print_msg_warn ();
  printf ("%s ", file);
  puts ("has been modified since last check, not removing");
  return true;
}
