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

#pragma once

#include "trashinfo_rmw.h"
#include "parse_cli_options.h"
#include "messages_rmw.h"
#include "canfigger.h"
#include "main.h"

#define ENV_RMW_FAKE_HOME "RMW_FAKE_HOME"

extern const char *expire_age_str;

typedef struct
{
  char uid[10];
  st_waste *st_waste_folder_props_head;
  bool force_required;
  bool fake_media_root;
  int expire_age;
} st_config;

void print_config(FILE * restrict stream);

void
parse_config_file(const rmw_options * cli_user_options,
                  st_config * st_config_data, const st_loc * st_location);

void init_config_data(st_config * x);

void
show_folder_line(const char *folder, const bool is_r, const bool is_attached);
