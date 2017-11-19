/*
 * messages_rmw.c
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

#include "messages_rmw.h"

/**
 * open_file()
 *
 * called if fopen() returns NULL. Print error message
 */
void
open_err (const char *filename, const char *function_name)
{
    printf (_("Error: while opening %s\n"), filename);

    char combined_msg[MAX_MSG_SIZE];
    sprintf (combined_msg, _("function: <%s>"), function_name);
    perror (combined_msg);

    return;
}

/**
 * close_file()
 *
 * Closes a file, checks for an error. If error print message and
 * return 1, else, returns 0
 */
short close_file (FILE *file_ptr, const char *filename, const char *function_name)
{
  if (fclose (file_ptr) != EOF)
    return 0;
  else
  {
    printf (_("Warning: while closing %s\n"), filename);

    char combined_msg[MAX_MSG_SIZE];
    sprintf (combined_msg, _("function: <%s>"), function_name);
    perror (combined_msg);

    return 1;
  }
}
