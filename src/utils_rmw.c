/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2022  Andy Alt (arch_stanton5995@protonmail.com)
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
char *
rmw_dirname(char *path)
{
  if (path == NULL || *path == '\0')
    return NULL;

  int len = strlen(path);
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

  if (isdotdir(path))
    if (path[1] == '.')
      path[1] = '\0';

  // No slashes were found
  if (ptr == &path[0])
    strcpy(path, ".");

  return path;
}


/**
 * rmw_mkdir()
 *
 * Check for the existence of a dir, and create it if not found.
 * Also creates parent directories.
 */
int
rmw_mkdir(const char *dir, mode_t mode)
{
  if (exists(dir))
  {
    errno = EEXIST;
    return -1;
  }

  int res = 0;

  char tmp[strlen(dir) + 1];
  strcpy(tmp, dir);
  char *parent = rmw_dirname(tmp);
  if (!parent)
    return -1;
  if (!exists(parent))
    res = rmw_mkdir(parent, mode);

  if (res)
    return res;

  return mkdir(dir, mode);
}


/*!
 * Determine whether or not a file or directory exists.
 */
bool
exists(const char *filename)
{
  if (!filename)
    return false;
  /* access() always dereferences symbolic links, and therefore doesn't
   * recognized broken links. */
  // return ! access (filename, F_OK);

  /* And we don't use fopen() because sometimes we want to know if the
     file exists, not just if it can be read. If a file or directory
     can be opened read-only, that doesn't guarantee whether or not it
     exists */

  static struct stat st;
  static int res;
  res = (lstat(filename, &st));
  if (!res)
    return true;

  // reset errno since we are only checking for the file existence
  errno = 0;
  return false;
}

void
dispose_waste(st_waste * node)
{
  if (node != NULL)
  {
    dispose_waste(node->next_node);
    free(node->parent);
    free(node->files);
    free(node->info);
    if (node->media_root != NULL)
      free(node->media_root);
    free(node);
  }
  return;
}

void
make_size_human_readable(const off_t size, char *buf)
{
  /* Store only the first letter; we add "iB" later during snprintf(). */
  const char prefix[] = { 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };

  short power = -1;
  off_t remainder = 0;
  off_t hr_size = size;

  while (hr_size >= 1024)
  {
    remainder = hr_size % 1024;
    hr_size /= 1024;
    power++;
  }

  // Doing some casting here because after the division above, 'hr_size'
  // and 'remainder' should not be any more than 4 digits
  if (power >= 0)
    sn_check(snprintf
             (buf, LEN_MAX_HUMAN_READABLE_SIZE, "%d.%d %ciB",
              (short) hr_size, (short) remainder * 10 / 1024,
              prefix[power]), LEN_MAX_HUMAN_READABLE_SIZE);
  else
    sn_check(snprintf
             (buf, LEN_MAX_HUMAN_READABLE_SIZE, "%d B",
              (short) hr_size), LEN_MAX_HUMAN_READABLE_SIZE);

  return;
}

/*!
 * Prompt user for confirmation.
 * @return true if 'y' or 'Y' was entered, false otherwise"
 */
bool
user_verify(void)
{
  fputs(_("Continue? (y/n): "), stdout);
  int answer = getchar();
  bool want_continue = (answer == (int) 'Y') || (answer == (int) 'y');
  int char_count = 0;

  /* Check if there's any more chars */
  while (answer != '\n' && answer != EOF)
  {
    char_count++;
    if (char_count > 1024)
    {
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
static bool
is_unreserved(char c)
{
  if (('A' <= c && c <= 'Z') ||
      ('a' <= c && c <= 'z') || ('0' <= c && c <= '9'))
    return 1;

  switch (c)
  {
  case '-':
  case '_':
  case '~':
  case '.':
  case '/':
    return 1;

  default:
    return 0;
  }
}


/*!
 * Convert str into a URL valid string, escaping when necessary
 * returns an allocated string which must be freed later
 */
char *
escape_url(const char *str)
{
  int pos_str = 0, pos_dest = 0;
  char *dest = malloc(LEN_MAX_ESCAPED_PATH);
  chk_malloc(dest, __func__, __LINE__);
  *dest = '\0';

  while (str[pos_str])
  {
    if (is_unreserved(str[pos_str]))
    {
      bufchk_len(pos_dest + 2, LEN_MAX_ESCAPED_PATH, __func__, __LINE__);
      dest[pos_dest] = str[pos_str];
      pos_dest += 1;
    }
    else
    {
      bufchk_len(pos_dest + 4, LEN_MAX_ESCAPED_PATH, __func__, __LINE__);
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
  return dest;
}


/**
 * Convert a URL valid string into a regular string, unescaping any '%'s
 * that appear.
 * returns an allocated string which must be freed later
 */
char *
unescape_url(const char *str)
{
  int pos_str = 0, pos_dest = 0;
  char *dest = malloc(LEN_MAX_PATH);
  chk_malloc(dest, __func__, __LINE__);

  while (str[pos_str])
  {
    if (str[pos_str] == '%')
    {
      /* skip the '%' */
      pos_str += 1;
      bufchk_len(pos_dest + 2, LEN_MAX_ESCAPED_PATH, __func__, __LINE__);
      // Is casting dest to unsigned char* ok here? Is there a better way to
      // do the conversion?
      sscanf(str + pos_str, "%2hhx", (unsigned char *) dest + pos_dest);
      pos_str += 2;
    }
    else
    {
      bufchk_len(pos_dest + 2, LEN_MAX_ESCAPED_PATH, __func__, __LINE__);
      dest[pos_dest] = str[pos_str];
      pos_str += 1;
    }
    pos_dest++;
  }
  dest[pos_dest] = '\0';
  return dest;
}


/*
 * name: isdotdir
 * Checks for . and .. directories
 */
bool
isdotdir(const char *dir)
{
  return (strcmp(dir, ".") == 0 || strcmp (dir, "..") == 0);
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
char *
resolve_path(const char *file, const char *b)
{
  int req_len = strlen(file) + 1;
  bufchk_len(req_len, LEN_MAX_PATH, __func__, __LINE__);
  char tmp[req_len];
  strcpy(tmp, file);

  char *orig_dirname = realpath(rmw_dirname(tmp), NULL);
  if (orig_dirname == NULL)
  {
    print_msg_error();
    perror("realpath()");
    return NULL;
  }

  char *abspath = join_paths(orig_dirname, b);
  free(orig_dirname);
  return abspath;
}


/*!
 * Trim a trailing character of a string
 * @param[in] c The character to erase
 * @param[out] str The string to alter
 * @return void
 */
void
trim_char(const int c, char *str)
{
  trim_whitespace(str);
  if (*str == '\0')
    return;
  while (*str != '\0')
    str++;

  str--;

  while (*str == c)
  {
    *str = '\0';
    str--;
  }

  return;
}


char *
real_join_paths(const char *argv, ...)
{
  char *path = calloc(1, LEN_MAX_PATH);
  chk_malloc(path, __func__, __LINE__);

  va_list ap;
  char *str = (char *) argv;
  va_start(ap, argv);

  while (str != NULL)
  {
    size_t len = 0;
    char *dup_str = strdup(str);
    chk_malloc(dup_str, __func__, __LINE__);
    trim_char('/', dup_str);
    len = strlen(path);
    int max_len = LEN_MAX_PATH - len;
    int r = snprintf(path + len, max_len, "%s/", dup_str);
    free(dup_str);
    sn_check(r, max_len);
    str = va_arg(ap, char *);
  }

  va_end(ap);
  trim_char('/', path);
  path = realloc(path, strlen(path) + 1);
  chk_malloc(path, __func__, __LINE__);
  return path;
}


///////////////////////////////////////////////////////////////////////
#ifdef TEST_LIB

#include "test.h"
#include "purging_rmw.h"


static void
test_isdotdir(void)
{
  assert(isdotdir(".") == true);
  assert(isdotdir("..") == true);
  assert(isdotdir(".t") == false);
  assert(isdotdir("...") == false);
  assert(isdotdir("t.") == false);
  assert(isdotdir(".. ") == false);

  return;
}


static void
test_rmw_mkdir(const char *h)
{
  const char *subdirs = "foo/bar/21/42";
  char *dir = join_paths(h, subdirs);
  assert(rmw_mkdir(dir, S_IRWXU) == 0);
  printf("%s\n", dir);
  assert(exists(dir) == true);

  st_rm rm;
  init_rm(&rm);
  char rm_cmd[LEN_MAX_RM_CMD];

  assert((size_t) snprintf(rm_cmd,
                           sizeof rm_cmd,
                           "%s -rf %s %s %s",
                           rm.full_path, rm.v, rm.onefs,
                           dir) < sizeof rm_cmd);
  assert(system(rm_cmd) == 0);
  free(dir);

  return;
}

static void
test_rmw_dirname(void)
{
  char dir[BUFSIZ];
  strcpy(dir, "/");
  assert(strcmp(rmw_dirname(dir), "/") == 0);

  strcpy(dir, "./foo");
  assert(strcmp(rmw_dirname(dir), ".") == 0);

  strcpy(dir, "../foo");
  assert(strcmp(rmw_dirname(dir), "..") == 0);

  strcpy(dir, "./foo/");
  assert(strcmp(rmw_dirname(dir), ".") == 0);

  strcpy(dir, "./foo/bar/");
  assert(strcmp(rmw_dirname(dir), "./foo") == 0);

  strcpy(dir, "foo/bar/42");
  assert(strcmp(rmw_dirname(dir), "foo/bar") == 0);

  strcpy(dir, "/foo/bar/42");
  assert(strcmp(rmw_dirname(dir), "/foo/bar") == 0);

  strcpy(dir, "..");
  assert(strcmp(rmw_dirname(dir), ".") == 0);

  strcpy(dir, ".");
  assert(strcmp(rmw_dirname(dir), ".") == 0);

  strcpy(dir, "usr");
  assert(strcmp(rmw_dirname(dir), ".") == 0);

  strcpy(dir, "/usr/");
  assert(strcmp(rmw_dirname(dir), "/") == 0);

  strcpy(dir, "//");
  assert(strcmp(rmw_dirname(dir), "/") == 0);

  strcpy(dir, "");
  assert(rmw_dirname(dir) == NULL);

  return;
}

static void
test_make_size_human_readable(void)
{
  struct expected
  {
    const off_t file_size;
    const char *out;
  } const data[] = {
    {256, "256 B"},
    {1024, "1.0 KiB"},
    {12000, "11.7 KiB"},
    {32000000000, "29.8 GiB"},
    {62000000000000, "56.3 TiB"},
    {82000300000000000, "72.8 PiB"},
  };

  int data_size = ARRAY_SIZE(data);
  int i = 0;
  while (i < data_size)
  {
    char hr[LEN_MAX_HUMAN_READABLE_SIZE];
    make_size_human_readable(data[i].file_size, hr);
    fprintf(stderr, "hr_size: %s\nExpected: %s\n\n", hr, data[i].out);
    assert(strcmp(hr, data[i].out) == 0);
    i++;
  }

  assert(i == data_size);

  return;
}


void
test_join_paths(void)
{
  char *path = join_paths("home", "foo//", "bar");
  assert(path != NULL);
  assert(strcmp(path, "home/foo/bar") == 0);
  free(path);

  path = join_paths("/home/foo", "bar", "world/");
  assert(path != NULL);
  assert(strcmp(path, "/home/foo/bar/world") == 0);
  free(path);

  return;
}

void
test_trim_char(void)
{
  char foo[] = "Hello Worldd    ";
  trim_char('d', foo);
  assert(strcmp(foo, "Hello Worl") == 0);
  *foo = '\0';
  trim_char('/', foo);
  assert(*foo == '\0');
  return;
}


int
main()
{
  char tmp[LEN_MAX_PATH];
  int r =
    snprintf(tmp, LEN_MAX_PATH, "%s/%s", RMW_FAKE_HOME, "test_utils_dir");
  assert(r < LEN_MAX_PATH);
  const char *HOMEDIR = tmp;

  test_isdotdir();
  test_rmw_mkdir(HOMEDIR);
  test_rmw_dirname();
  test_make_size_human_readable();
  test_join_paths();
  test_trim_char();

  char *str = "reserved    = ; | / | ? | : | @ | & | = | + | $ \n\t\v  \f\r";
  char *escaped_path = escape_url(str);
  fprintf(stderr, "'%s'\n", escaped_path);
  assert(!strcmp
         (escaped_path,
          "reserved%20%20%20%20%3D%20%3B%20%7C%20/%20%7C%20%3F%20%7C%20%3A%20%7C%20%40%20%7C%20%26%20%7C%20%3D%20%7C%20%2B%20%7C%20%24%20%0A%09%0B%20%20%0C%0D"));

  char *unescaped_path = unescape_url(escaped_path);
  fprintf(stderr, "'%s'\n", unescaped_path);
  assert(!strcmp(unescaped_path, str));

  free(unescaped_path);
  free(escaped_path);
  return 0;
}
#endif
