/*
 * primary_funcs.c
 *
 * This file is part of rmw (http://rmw.sf.net)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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

int trim (char s[]);

void erase_char (char c, char *str);

bool change_HOME (char *t, const char *HOMEDIR);

int trim_slash (char s[]);

void truncate_str (char *str, short len);

bool
pre_rmw_check (const char *cmdargv, char *file_basename, char *cur_file,
               struct waste_containers *waste, bool bypass);

int
mkinfo (bool dup_filename, char *file_basename, char *cur_file,
        struct waste_containers *waste, char *time_now,
        char *time_str_appended);

int
purge (int purge_after, struct waste_containers *waste, char *time_now);

bool purgeD (const char *HOMEDIR);

void
undo_last_rmw (const char *HOMEDIR, char *time_str_appended);

int getch (void);

/* reads from keypress, echoes */
int getche (void);

/* Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 * */
bool buf_check_with_strop (char *s1, const char *s2, bool mode);

bool buf_check (const char *str, unsigned short boundary);

int rmdir_recursive (char *path, bool isTop);

void mkdirErr (const char *dirname, const char *text_string);

void waste_check (const char *p);

bool
isProtected (char *cur_file, struct waste_containers *waste, bool bypass);

void
Restore (int argc, char *argv[], int optind, char *time_str_appended);

void
restore_select (struct waste_containers *waste, char *time_str_appended);

bool file_exist (const char *filename);

void get_time_string (char *tm_str, unsigned short len, const char *format);

int
remove_to_waste(char *cur_file, char *file_basename, struct waste_containers *waste,
                char *time_now, char *time_str_appended, FILE *undo_file_ptr);

void print_usage (void);
void warranty (void);
void version (void);
