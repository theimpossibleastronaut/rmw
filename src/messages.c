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

#include "messages.h"

#include <stdarg.h>

void
diag(diag_level level, const char *fmt, ...)
{
  switch (level)
  {
  case DIAG_ERR:
    fputs(_("  :error: "), stderr);
    break;
  case DIAG_WARN:
    fputs(_(" :warning: "), stderr);
    break;
  }

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void
diag_fatal(const int exit_code, const char *fmt, ...)
{
  fputs(_("  :error: "), stderr);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  exit(exit_code);
}

/**
 * Called if fopen() returns NULL. prints an error message and some
 * extra info about the cause.
 * @param[in] filename the name of the file
 * @param[in] function_name the name of the function where the error originated
 * @return void
 */
void
open_err(const char *filename, const char *function_name)
{
  /* TRANSLATORS:  "opening" refers to a file  */
  diag(DIAG_ERR, _("while opening %s\n\
function: <%s>\n\
%s\n"), filename, function_name, strerror(errno));

  return;
}

/**
 * Closes a file, checks for an error. If one is returned, print a message
 * with some extra info about the error.
 *
 * @param[in] file_ptr a file pointer that already exists
 * @param[in] filename
 * @param[in] function_name the name of the calling function
 * @return an error number, 0 if no error
 */
int
close_file(FILE **fp, const char *filename, const char *function_name)
{
  if (*fp == NULL)
    return -1;

  if (fclose(*fp) != EOF)
    return 0;
  else
  {
    int dup_errno = errno;
    /* TRANSLATORS:  "closing" refers to a file  */
    diag(DIAG_ERR, _("while closing %s\n\
function: <%s>\n\
%s\n"), filename, function_name, strerror(dup_errno));
    return dup_errno;
  }
}


void
display_dot_trashinfo_error(const char *dti)
{
  /* TRANSLATORS:  ".trashinfo" should remain untranslated
   *
   *               "format" refers to the layout of the file
   *                contents
   */
  diag(DIAG_ERR, _("format of .trashinfo file '%s' is incorrect\n"), dti);
  return;
}


void
real_fatal_malloc(const char *func, const int line)
{
  int save_errno = errno;
  fprintf(stderr, "%s -- %s:L%d\n", strerror(errno), func, line);
  exit(save_errno);
}


void
msg_err_close_dir(const char *dir, const char *func, const int line)
{
  diag(DIAG_ERR, "while closing %s -- %s:L%d\n", dir, func, line);
  perror("closedir()");
  exit(errno);
}

void
msg_err_open_dir(const char *dir, const char *func, const int line)
{
  diag(DIAG_ERR, _("while opening %s -- %s:L%d\n%s\n"), dir, func, line,
       strerror(errno));
  return;
}

void
msg_err_rename(const char *src_file, const char *dest_file)
{
  diag_fatal(EXIT_FAILURE, "%s -> %s\n\
rename: %s\n", src_file, dest_file, strerror(errno));
}

/*!
 * Used for error-checking calls to fprintf.
 * @param[in] func The function in which the error occurred
 * @return exit failure
 */
void
msg_err_fatal_fprintf(const char *func)
{
  diag_fatal(EXIT_FAILURE, "fprintf returned an error in %s.\n", func);
}

/*!
 * Used by functions that prevent buffer overflows
 * @param[in] func the function from which it was originally called
 * @param[in] line the line number of the original calling function
 * @return void
 * @see bufchk
 * @see bufchk_len
 */
void
msg_err_buffer_overrun(const char *func, const int line)
{
  diag(DIAG_ERR, "func = %s:L%d\n", func, line);
  /* TRANSLATORS:  "buffer" in the following instances refers to the amount
   * of memory allocated for a string  */
  fputs("Buffer length exceeded.\n", stderr);
  fputs
    ("If you think this may be a bug, please report it to the rmw developers.\n",
     stderr);
}

/*!
 * Called if lstat() returns an error.
 * @param[in] func the name of the calling function
 * @param[in] line the line number from where the function was called
 * @return void
 */
void
msg_err_lstat(const char *file, const char *func, const int line)
{
  int dup_errno = errno;
  diag_fatal(dup_errno, "lstat(): %s\n%s in %s:L%d\n", strerror(dup_errno),
             file, func, line);
}

void
msg_err_remove(const char *file, const char *func)
{
  int dup_errno = errno;
  diag(DIAG_ERR, _("while removing %s:\n%s\n(func:%s)\n"), file,
       strerror(dup_errno), func);
}


void
msg_err_mkdir(const char *dir, const char *func)
{
  perror("rmw_mkdir()");
  diag(DIAG_ERR, _("while creating %s (%s)\n"), dir, func);
}

void
msg_success_mkdir(const char *dir)
{
  printf(_("Created directory %s\n"), dir);
  return;
}


void
msg_warn_file_not_found(const char *file)
{
  printf("'%s': %s\n", file, strerror(ENOENT));
}

void
verbose_printf(const int min_level, const char *fmt, ...)
{
  if (verbose < min_level)
    return;

  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
}
