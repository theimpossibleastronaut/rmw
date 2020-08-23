/*
 * messages_rmw.c
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

#include "messages_rmw.h"

/* These strings need to match the order listed in the enum statement
 * in rmw.h */
static const char *ERR_STRING[] = {
  "EXIT_FAILED_GETENV",
  "NO_WASTE_FOLDER",
  "EXIT_BUF_ERR",
  "EXIT_MALLOC_ERR",
  "MAKE_DIR_SUCCESS",
  "MAKE_DIR_FAILURE",
  "ERR_OPEN",
  "ERR_CLOSE",
  "ERR_FGETS",
  "ERR_TRASHINFO_FORMAT",
  "FILE_NOT_FOUND",
  "ERR_LSTAT"
};

void print_msg_error (void)
{
  /* TRANSLATORS: this precedes a string which informs the user of a more
   * serious error, sometimes a fatal one */
  fputs (_("  :error: "), stderr);
}

void print_msg_warn (void)
{
  /* TRANSLATORS: this precedes a string which warns the user of some minor
   * but not fatal problem */
  fputs (_(" :warning: "), stdout);
}

/**
 * Called if fopen() returns NULL. prints an error message and some
 * extra info about the cause.
 * @param[in] filename the name of the file
 * @param[in] function_name the name of the function where the error originated
 * @return void
 */
void
open_err (const char *filename, const char *function_name)
{
    print_msg_error ();
    /* TRANSLATORS:  "opening" refers to a file  */
    printf (_("while opening %s\n"), filename);

    static char combined_msg[MAX_MSG_SIZE];
     /* TRANSLATORS:  "function" refers to a C function  */
    sprintf (combined_msg, _("function: <%s>"), function_name);
    perror (combined_msg);

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
short close_file (FILE *file_ptr, const char *filename, const char *function_name)
{
  /* fclose() shouldn't be used on file pointers that are NULL.
   * https://stackoverflow.com/questions/16922871/why-glibcs-fclosenull-cause-segmentation-fault-instead-of-returning-error
   */
  if (file_ptr == NULL)
    return 0;

  if (fclose (file_ptr) != EOF)
    return 0;
  else
  {
    /* TRANSLATORS:  "closing" refers to a file  */
    print_msg_error ();
    printf (_("while closing %s\n"), filename);

    static char combined_msg[MAX_MSG_SIZE];
    sprintf (combined_msg, _("function: <%s>"), function_name);
    perror (combined_msg);

    msg_return_code (ERR_CLOSE);
    return ERR_CLOSE;
  }
}

void display_dot_trashinfo_error (const char *dti)
{
  print_msg_error ();
  /* TRANSLATORS:  ".trashinfo" should remain untranslated
   *
   *               "format" refers to the layout of the file
   *                contents
   */
  printf (_("format of .trashinfo `file %s` is incorrect"), dti);
  printf ("\n");
  return;
}

void msg_warn_restore (int result)
{
  if (result == 0)
    return;

  if (result != FILE_NOT_FOUND)
  {
    print_msg_warn ();
    if (result != FILE_NOT_FOUND)
    {
      /* TRANSLATORS: ignore "restore()"
       * "returned" refers to an error code (number) that was returned by
       * an operation */
      printf (_("restore() returned %d\n"), result);
    }
  }

  return;
}

void chk_malloc (void *state, const char *func, const int line)
{
  if (state == NULL)
  {
    print_msg_error ();
    printf (_("while attempting to allocate memory -- %s:L%d\n"), func, line);
    msg_return_code (EXIT_MALLOC_ERR);
    exit (EXIT_MALLOC_ERR);
  }
  return;
}

void
msg_return_code (int code)
{
  if (verbose)
  {
    /* TRANSLATORS: "return" code refers to a number returned by an operation
     * or function */
    printf (_("  :return code: %d -- %s\n"), code, ERR_STRING[code - RETURN_CODE_OFFSET]);
  }
}

void
msg_err_close_dir (const char *dir, const char *func, const int line)
{
  print_msg_error ();
  fprintf (stderr, "while closing %s -- %s:L%d\n", dir, func, line);
  perror ("closedir()");
  exit (errno);
}

void
msg_err_open_dir (const char *dir, const char *func, const int line)
{
  print_msg_error ();
  fprintf (stderr, _("while opening %s -- %s:L%d\n"), dir, func, line);
  perror ("opendir()");
  exit (errno);
}

void
msg_err_rename (const char *src_file, const char *dest_file, const char *func, const int line)
{
  print_msg_error ();
  printf (_("while trying to move (rename)\n\
  %s -> %s -- %s:L%d\n"), src_file, dest_file, func, line);
  perror ("rename()");
  exit (errno);
}

/*!
 * Used for error-checking calls to fprintf.
 * @param[in] func The function in which the error ocurred
 * @return exit failure
 */
void
msg_err_fatal_fprintf (const char *func)
{
  print_msg_error ();
  fprintf (stderr, "fprintf returned an error in %s.\n", func);
  exit (EXIT_FAILURE);
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
msg_err_buffer_overrun (const char *func, const int line)
{
  fprintf (stderr, "func = %s:L%d\n", func, line);
  /* TRANSLATORS:  "buffer" in the following instances refers to the amount
   * of memory allocated for a string  */
  fputs ("buffer overrun (segmentation fault) prevented.\n", stderr);
  fputs ("If you think this may be a bug, please report it to the rmw developers.\n", stderr);
}

/*!
 * Called if lstat() returns an error.
 * @param[in] func the name of the calling function
 * @param[in] line the line number from where the function was called
 * @return void
 */
void
msg_err_lstat (const char *file, const char *func, const int line)
{
  print_msg_error();
  perror ("lstat()");
  fprintf (stderr, "%s in %s:L%d\n", file, func, line);
  exit (ERR_LSTAT);
}

void
msg_err_remove (const char *file, const char *func)
{
  print_msg_error ();
  /* TRANSLATORS:  "removing" refers to a file or folder  */
  fprintf (stderr, _("while removing %s\n"), file);
  perror (func);
}



