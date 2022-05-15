/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2021  Andy Alt (arch_stanton5995@protonmail.com)
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

#include <ctype.h>
#include "strings_rmw.h"
#include "messages_rmw.h"
#include "utils_rmw.h"

/*!
 * Verify that len doesn't exceed boundary, otherwise exit with an error code.
 * Usually used before concatenating 2 or more strings. Program will exit
 * with an error code if len exceeds boundary. len should already have space
 * for the null terminator when this function is called.
 */
void
bufchk_len(const size_t len, const size_t dest_boundary, const char *func,
           const int line)
{
  if (len <= dest_boundary)
    return;

  msg_err_buffer_overrun(func, line);
#ifndef TEST_LIB
  exit(EBUF);
#endif
  errno = 1;
  return;
}

void
sn_check(const size_t len, const size_t dest_boundary, const char *func,
         const int line)
{
  if (len < dest_boundary)
    return;

  msg_err_buffer_overrun(func, line);

#ifndef TEST_LIB
  exit(EBUF);
#endif
  errno = 1;
  return;
}

/*!
 * Get the combined length of multiple strings. Last argument MUST be "NULL".
 *
 * @param[in] argv The first argument
 * @param[in] ... variable argument list, each argument must be a string.
 * @return the combined length of each string
 */
size_t
multi_strlen(const char *argv, ...)
{
  va_list vlist;
  char *str;
  size_t len = 0;

  str = (char *) argv;
  va_start(vlist, argv);
  while (str != NULL)
  {
    len += strlen(str);
    str = va_arg(vlist, char *);
  }

  va_end(vlist);
  return len;
}

/*!
 * Removes trailing white space from a string (including newlines, formfeeds,
 * tabs, etc
 * @param[out] str The string to be altered
 * @return void
 */
void
trim_whitespace(char *str)
{
  if (str == NULL)
  {
#ifndef TEST_LIB
    print_msg_error();
    fprintf(stderr, "%s received a NULL", __func__);
    exit(EXIT_FAILURE);
#else
    errno = 1;
    return;
#endif
  }

  char *pos_0 = str;
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
    str++;

  /* set pointer to segment preceding NULL terminator */
  if (str != pos_0)
    str--;
  else
    return;

  while (isspace(*str))
  {
    *str = '\0';
    if (str != pos_0)
      str--;
    else
      break;
  }

  return;
}


/**
 * Add a null terminator to chop off part of a string
 * at a given position
 * @param[in] pos The position at which to add the \0 character
 * @param[out] str The string to change
 * @return void
 */
void
truncate_str(char *str, const size_t pos)
{
  str[strlen(str) - pos] = '\0';
}

///////////////////////////////////////////////////////////////////////
#ifdef TEST_LIB

#include "test.h"

#define BUF_SIZE 80

static void
test_multi_strlen(void)
{
  assert(multi_strlen("this", " is", " a", " test string", NULL) ==
         strlen("this is a test string"));
  return;
}

static void
test_bufchk_len(void)
{
  errno = 0;
  bufchk_len(12, 12, __func__, __LINE__);
  assert(!errno);

  bufchk_len(12, 11, __func__, __LINE__);
  assert(errno);
  errno = 0;
  return;
}

static void
test_sn_check(void)
{
  errno = 0;
  sn_check(24, 24, __func__, __LINE__);
  assert(errno);
  errno = 0;

  sn_check(33, 34, __func__, __LINE__);
  assert(!errno);
  return;
}

static void
test_trim_whitespace()
{
  // handle strings that are NULL
  errno = 0;
  char *test = NULL;
  trim_whitespace(test);
  assert(errno == 1);
  errno = 0;

  test = calloc(1, BUF_SIZE + 1);
  chk_malloc(test, __func__, __LINE__);

  // handle strings that are empty
  test[0] = '\0';
  trim_whitespace(test);
  assert(strcmp(test, "") == 0);

  strcpy(test, " \n\t\v\f\r ");
  trim_whitespace(test);
  assert(!strcmp(test, ""));

  /* fails if \b is present */
  strcpy(test, "Hello World\n\t\v\f\r ");
  trim_whitespace(test);
  printf("'%s'\n", test);
  assert(!strcmp(test, "Hello World"));

  strcpy(test, "Hello World\n\t\v stop\f\r ");
  trim_whitespace(test);
  assert(!strcmp(test, "Hello World\n\t\v stop"));

  free(test);
}


int
main()
{
  test_trim_whitespace();
  test_bufchk_len();
  test_sn_check();
  test_multi_strlen();

  return 0;
}

#endif
