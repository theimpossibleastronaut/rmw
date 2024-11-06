/*
This file is part of rmw<https://theimpossibleastronaut.github.io/rmw-website/>

Copyright (C) 2012-2023  Andy Alt (arch_stanton5995@proton.me)
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

#include "utils.h"
#include "messages.h"

bool
is_symlink(const char *path)
{
  int fd = open(path, O_RDONLY | O_NOFOLLOW);
  if (fd != -1)
  {
    close(fd);
    return false;
  }
  else if (errno == ELOOP)
    return true;

  return false;
}


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
  if (ptr == &path[0] && len >= 1)
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
rmw_mkdir(const char *dir)
{
  if (!dir)
    return -1;
  if (check_pathname_state(dir) == P_STATE_EXISTS)
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
  int p_state = check_pathname_state(parent);
  if (p_state == P_STATE_ENOENT)
    res = rmw_mkdir(parent);
  else if (p_state == P_STATE_ERR)
    exit(p_state);

  if (res)
    return res;

  return mkdir(dir, 0777);
}


/*!
 * Determine whether a file or directory exists, and is accessible.
 */
int
check_pathname_state(const char *pathname)
{
  if (!pathname)
    return false;

  int fd = open(pathname, O_RDONLY);
  if (fd != -1)
  {
    if (close(fd) == -1)
    {
      perror("close");
      errno = 0;
    }
    return P_STATE_EXISTS;
  }

  // open() returns ENOENT in the case of dangling symbolic links
  // check if it's a link or not:
  if (errno == ENOENT)
  {
    static char buf[1];
    ssize_t f = readlink(pathname, buf, 1);
    *buf = '\0';
    if (f != -1)
      return P_STATE_EXISTS;
    else
    {
      errno = 0;
      return P_STATE_ENOENT;
    }
  }

  printf("open %s: %s\n", pathname, strerror(errno));
  errno = 0;
  return P_STATE_ERR;
}

void
dispose_waste(st_waste *node)
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
  * - The following characters: ~ _ - .
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
  if (!dest)
    fatal_malloc();
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
  char *dest = malloc(PATH_MAX);
  if (!dest)
    fatal_malloc();

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
  return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);
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
  bufchk_len(req_len, PATH_MAX, __func__, __LINE__);
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
  char *dup_str = str;
  trim_whitespace(dup_str);
  if (*dup_str == '\0')
    return;

  while (*dup_str != '\0')
    dup_str++;

  dup_str--;

  while (*dup_str == c)
  {
    *dup_str = '\0';
    if (dup_str == str)
      return;
    dup_str--;
  }

  return;
}


char *
real_join_paths(const char *argv, ...)
{
  char *path = calloc(1, PATH_MAX);
  if (!path)
    fatal_malloc();

  va_list ap;
  char *str = (char *) argv;
  va_start(ap, argv);

  while (str != NULL)
  {
    size_t len = 0;
    char *dup_str = strdup(str);
    if (!dup_str)
      fatal_malloc();
    trim_char('/', dup_str);
    len = strlen(path);
    int max_len = PATH_MAX - len;
    int r = snprintf(path + len, max_len, "%s/", dup_str);
    free(dup_str);
    sn_check(r, max_len);
    str = va_arg(ap, char *);
  }

  va_end(ap);
  trim_char('/', path);
  if (!(path = realloc(path, strlen(path) + 1)))
    fatal_malloc();
  return path;
}

bool
is_dir_f(const char *pathname)
{
  // O_FOLLOW is needed, otherwise the a symlink to a directory
  // would be detected as a directory, which we don't want
  int fd = open(pathname, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
  if (fd != -1)
  {
    if (close(fd) != 0)
      perror("close:");
    return true;
  }
  return false;
}

int
count_chars(const char c, const char *str)
{
  int i = 0, n = 0;
  while (str[i])
  {
    if (str[i++] == c)
      n++;
  }
  return n;
}


///////////////////////////////////////////////////////////////////////
#ifdef TEST_LIB

#include "test.h"
#include "purging.h"


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
  if (check_pathname_state(dir) == P_STATE_EXISTS)
    assert(bsdutils_rm(dir, verbose) == 0);
  assert(rmw_mkdir(dir) == 0);
  assert(dir);
  printf("%s\n", dir);
  assert(check_pathname_state(dir) == P_STATE_EXISTS);
  assert(bsdutils_rm(dir, verbose) == 0);
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
  strcpy(foo, "ccc");
  trim_char('c', foo);
  assert(*foo == '\0');
  return;
}


static void
test_is_dir_f(const char *const homedir)
{
  assert(is_dir_f("."));
  char *foobar = join_paths(homedir, "foobar");
  FILE *fp = fopen(foobar, "w");
  assert(fp != NULL);
  assert(fclose(fp) != EOF);

  assert(!is_dir_f(foobar));
  char *snafu = join_paths(homedir, "snafu");
  assert(symlink(foobar, snafu) == 0);
  assert(!is_dir_f(snafu));
  assert(bsdutils_rm(foobar, false) == 0);
  free(foobar);
  assert(bsdutils_rm(snafu, false) == 0);
  free(snafu);

  char *home_link = join_paths(homedir, "home_link");
  assert(symlink(homedir, home_link) == 0);
  assert(is_dir_f(home_link) == false);
  assert(bsdutils_rm(home_link, false) == 0);
  free(home_link);

  return;
}


static void
test_check_pathname_state(const char *const homedir)
{
  char *foobar = join_paths(homedir, "foobar");
  FILE *fp = fopen(foobar, "w");
  assert(fp != NULL);
  assert(fclose(fp) != EOF);
  assert(check_pathname_state(foobar) == P_STATE_EXISTS);

  char *snafu = join_paths(homedir, "snafu");
  assert(symlink(foobar, snafu) == 0);
  assert(check_pathname_state(snafu) == P_STATE_EXISTS);
  assert(remove(foobar) == 0);
  free(foobar);
  assert(remove(snafu) == 0);
  free(snafu);

  char *home_link = join_paths(homedir, "home_1234");
  assert(check_pathname_state(homedir) == P_STATE_EXISTS);
  if (check_pathname_state(home_link) == P_STATE_EXISTS)
    assert(remove(home_link) == 0);
  assert(symlink(homedir, home_link) == 0);
  assert(check_pathname_state(home_link) == P_STATE_EXISTS);
  assert(remove(home_link) == 0);
  free(home_link);

  const char *dlink = "dangling_link";
  if (check_pathname_state(dlink) == P_STATE_EXISTS)
    assert(remove(dlink) == 0);
  assert(symlink("dangler", dlink) == 0);
  assert(check_pathname_state(dlink) == P_STATE_EXISTS);
  assert(remove(dlink) == 0);

  return;
}


static void
test_count_chars(void)
{
  int n = 0;
  char *greet = "/hello/world";
  n = count_chars('/', greet);
  assert(n == 2);
  n = count_chars('l', greet);
  assert(n == 3);
  n = count_chars(' ', greet);
  assert(n == 0);
  return;
}


int
main()
{
  char tmp[PATH_MAX];
  int r = snprintf(tmp, PATH_MAX, "%s/%s", RMW_FAKE_HOME, "test_utils_dir");
  assert(r < PATH_MAX);
  const char *HOMEDIR = tmp;

  test_isdotdir();
  test_rmw_mkdir(HOMEDIR);
  test_rmw_dirname();
  test_make_size_human_readable();
  test_join_paths();
  test_trim_char();
  test_check_pathname_state(HOMEDIR);
  test_is_dir_f(HOMEDIR);

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

  test_count_chars();
  return 0;
}
#endif
