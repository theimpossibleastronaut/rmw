/*
 * utils_rmw.c
 *
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
  #include "globals.h"
#endif

#include "utils_rmw.h"
#include "strings_rmw.h"
#include "messages_rmw.h"

/*
 * name: rmw_dirname
 * return a pointer to the parent directory of *path
 * (mimics the behavior of dirname())
 *
 * This function may alter path
 *
 * (Using dirname() was causing errors on osx and OpenBSD 6.5
 * https://travis-ci.com/github/theimpossibleastronaut/rmw/builds/224722056
 * -andy5995 2021-05-02)
 */
char *rmw_dirname (char *path)
{
  if (path == NULL || *path == '\0')
    return NULL;

  int len = strlen (path);
  if (len > 1 && path[len - 1] == '/')
  {
    path[len - 1] = '\0';
    len--;
  }

  char *ptr = path + len - 1;

  while (*ptr != '/' && ptr != &path[0])
    ptr--;

  if (*ptr == '/')
  {
    if (len > 1)
    {
      if (ptr != &path[0])
      {
        *ptr = '\0';
      }
      else
      {
        path[1] = '\0';
      }
    }
    return path;
  }

  if (isdotdir (path))
    if (path[1] == '.')
      path[1] = '\0';

  // No slashes were found
  if (ptr == &path[0])
    strcpy (path, ".");

  return path;
}


/**
 * rmw_mkdir()
 *
 * Check for the existence of a dir, and create it if not found.
 * Also creates parent directories.
 */
int rmw_mkdir (const char *dir, mode_t mode)
{
  if (exists (dir))
  {
    errno = EEXIST;
    return -1;
  }

  int res = 0;

  char tmp[strlen (dir) + 1];
  strcpy (tmp, dir);
  char *parent = rmw_dirname (tmp);
  if (!exists (parent))
    res = rmw_mkdir (parent, mode);

  if (res)
    return res;

  return mkdir (dir, mode);
}


/*!
 * Determine whether or not a file or directory exists.
 */
bool exists (const char *filename)
{
  /* access() always dereferences symbolic links, and therefore doesn't
   * recognized broken links. */
  // return ! access (filename, F_OK);

  static struct stat st;
  static int res;
  res = (lstat (filename, &st));
  if (!res)
    return true;

  // reset errno since we are only checking for the file existence
  errno = 0;
  return false;
}

void
dispose_waste (st_waste *node)
{
  if (node != NULL)
  {
    dispose_waste (node->next_node);
    free (node->parent);
    free (node->files);
    free (node->info);
    if (node->media_root != NULL)
      free (node->media_root);
    free (node);
  }
  return;
}

char *
human_readable_size (off_t size)
{
  char *buffer = malloc (LEN_MAX_HUMAN_READABLE_SIZE);
  chk_malloc (buffer, __func__, __LINE__);

  /* Store only the first letter; we add "iB" later during snprintf(). */
  const char prefix[] = { 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
  short power = -1;

  short remainder;
  remainder = 0;

  while (size >= 1024)
  {
    remainder = size % 1024;
    size /= 1024;

    ++power;
  }

  char a_size[LEN_MAX_HUMAN_READABLE_SIZE];
  sprintf (a_size, "%d", (short)size);
  if (power >= 0)
    sprintf (buffer, "%s.%d %ciB", a_size,
              (remainder * 10) / 1024, prefix[power]);
  else
    sprintf (buffer, "%s B", a_size);

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

  return 0;
}


/**
 * Convert a URL valid string into a regular string, unescaping any '%'s
 * that appear.
 * returns 0 on succes, 1 on failure
 */
bool
unescape_url (const char *str, char *dest, const int len)
{
  int pos_str;
  int pos_dest;
  pos_str = 0;
  pos_dest = 0;

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
        printf (_
                ("rmw: %s(): buffer too small (got %d, needed a minimum of %d)\n"),
                __func__, len, pos_dest + 2);
        return 1;
      }

      sscanf (str + pos_str, "%2hhx", dest + pos_dest);
      pos_str += 2;
    }
    else
    {
      /* Check for buffer overflow (there should be enough space for 1
       * character + '\0') */
      if (pos_dest + 2 > len)
      {
        printf (_
                ("rmw: %s(): buffer too small (got %d, needed a minimum of %d)\n"),
                __func__, len, pos_dest + 2);
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


/*
 * name: isdotdir
 * Checks for . and .. directories
 */
bool isdotdir (const char *dir)
{
  if (dir[0] != '.')
    return false;
  if (dir[1] == '\0' || (dir[1] == '.' && dir[2] == '\0'))
    return true;

  return false;
}


/*
 * name: resolve_path()
 *
 * Gets the absolute path + filename
 *
 * realpath() by itself doesn't give the desired result if basename is
 * a dangling  * symlink; this function is designed to do that.
 *
 */
char
*resolve_path (const char *file, const char *b)
{
  int req_len = strlen (file) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char tmp[req_len];
  strcpy (tmp, file);

  char *orig_dirname = realpath (rmw_dirname (tmp), NULL);
  if (orig_dirname == NULL)
  {
    print_msg_error ();
    perror ("realpath()");
    return NULL;
  }

  req_len = multi_strlen (orig_dirname, "/", b, NULL) + 1;
  bufchk_len (req_len, LEN_MAX_PATH, __func__, __LINE__);
  char *abspath = malloc (req_len);
  chk_malloc (abspath, __func__, __LINE__);
  sprintf (abspath, "%s/%s", orig_dirname, b);
  free (orig_dirname);
  return abspath;
}
