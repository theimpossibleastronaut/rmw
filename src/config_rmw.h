/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_CONFIG_H
#define _INC_CONFIG_H

#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h> /* for geteuid() */

#include "trashinfo_rmw.h"
#include "parse_cli_options.h"
#include "messages_rmw.h"

#define LEN_MAX_CFG_LINE (LEN_MAX_PATH * 2 + 1)

/* Not currently used
 * #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
 */

typedef struct {
  const char *dir;
  char file[LEN_MAX_PATH];
  st_waste *st_waste_folder_props_head;
  int purge_after;
  bool force_required;
  bool fake_media_root;
  char *logfile;
  char *log_date;
} st_config;

struct st_vars_to_check {
  const char *name;
  const char *value;
};

const char*
get_config_home_dir (void);

void
parse_config_file (const rmw_options * cli_user_options, st_config *st_config_data);

void
init_config_data (st_config *st_config_data);

void
show_folder_line (const char *folder, const bool is_r, const bool is_attached);

#ifdef TEST_LIB
char *
del_char_shift_left (const char needle, char *haystack);

char
*strrepl (char *src, const char *str, char *repl);

void
realize_config_vars (char *str);

st_waste *
parse_line_waste (st_waste * waste_curr, const char * line_ptr,
                 const rmw_options * cli_user_options, bool fake_media_root);
#endif

#endif
