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

#pragma once

#include "trashinfo.h"
#include "parse_cli_options.h"
#include "messages.h"
#include <canfigger.h>
#include "main.h"

/* Discovery scans real mount points. It is on by default; a test that does
 * not isolate the host's mounts sets RMW_DISCOVERY=off to suppress it, while
 * a test that controls its own mounts (e.g. in a private mount namespace)
 * leaves it on. RMW_DISCOVERY is authoritative whenever it is set.
 *
 * Deprecated, honored only when RMW_DISCOVERY is unset: RMW_FAKE_HOME being
 * set implies discovery off, and RMW_CHECK_DISCOVERY forces it on. Prefer
 * RMW_DISCOVERY; the old pair will be removed in a future release. */
#define ENV_RMW_DISCOVERY "RMW_DISCOVERY"
#define ENV_RMW_FAKE_HOME "RMW_FAKE_HOME"
#define ENV_RMW_CHECK_DISCOVERY "RMW_CHECK_DISCOVERY"

extern const char *expire_age_str;

typedef struct
{
  st_waste *st_waste_folder_props_head; // Pointer with high alignment requirements

  int expire_age;               // 4-byte alignment, placed next

  bool force_required;          // Minimal alignment, placed after the int
  char uid[10];                 // Character array, lower alignment, placed last
} st_config;


void print_config(FILE * restrict stream);

void
parse_config_file(const rmw_options * cli_user_options,
                  st_config * st_config_data, const st_loc * st_location);

void init_config_data(st_config * x);

void
show_folder_line(const char *folder, const bool is_r, const bool is_attached,
                 const bool no_add);
