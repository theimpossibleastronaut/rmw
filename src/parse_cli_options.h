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

#include <getopt.h>

/** CLI option switches for rmw. */
typedef struct
{
  int want_purge;
  int force;

  /*! Alternate configuration file given at the command line with -c */
  const char *alt_config_file;

  bool want_restore;
  bool want_empty_trash;
  bool want_top_level_bypass;
  bool want_orphan_chk;
  bool want_selection_menu;
  bool want_undo;
  bool most_recent_list;
  bool want_dry_run;

  /*! list waste folder option */
  bool list;
} rmw_options;



void init_rmw_options(rmw_options * options);

void
parse_cli_options(const int argc, char *const argv[], rmw_options * options);
