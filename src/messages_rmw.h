/**
 * @file messages_rmw.h
 */
/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
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

#include "rmw.h"
#include "strings_rmw.h"

#define MAX_MSG_SIZE 512

void print_msg_error (void);

void print_msg_warn (void);

void
open_err (const char *filename, const char *function_name);

short
close_file (FILE *file_ptr, const char *filename, const char *function_name);

void display_dot_trashinfo_error (const char *dti);

void msg_warn_restore (int result);

void chk_malloc (void *state, const char *func, const int line);

void
msg_return_code (int code);

void
msg_err_close_dir (const char *dir, const char *func, const int line);

void
msg_err_open_dir (const char *dir, const char *func, const int line);

void
msg_err_rename (const char *src_file, const char *dest_file, const char *func, const int line);

void msg_err_fatal_fprintf (const char *func);
