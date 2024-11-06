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

#ifndef _INC_UTILS_H
#define _INC_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "trashinfo.h"

enum
{
  P_STATE_ERR = -1,
  P_STATE_ENOENT,
  P_STATE_EXISTS,
  P_STATE_SYMLINK
};

#define join_paths(...) real_join_paths(__VA_ARGS__, NULL)

#define LEN_MAX_HUMAN_READABLE_SIZE (sizeof "xxxx.y GiB")
#define LEN_MAX_FILE_DETAILS (LEN_MAX_HUMAN_READABLE_SIZE + sizeof "[] (D)" - 1)

bool is_symlink(const char *path);

char *rmw_dirname(char *path);

int rmw_mkdir(const char *dir);

int make_dir(const char *dir);

int check_pathname_state(const char *pathname);

void dispose_waste(st_waste * node);

void make_size_human_readable(off_t size, char *dest_hr_size);

bool user_verify(void);

char *escape_url(const char *str);

char *unescape_url(const char *str);

bool isdotdir(const char *dir);

char *resolve_path(const char *file, const char *b);

void trim_char(const int c, char *str);

char *real_join_paths(const char *argv, ...);

bool is_dir_f(const char *pathname);

int count_chars(const char c, const char *str);

#endif
