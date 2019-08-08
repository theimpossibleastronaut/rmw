/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2019 Andy Alt (andy400-dev@yahoo.com)
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

#ifndef _INC_PARSE_CLI_OPTIONS_H
#define _INC_PARSE_CLI_OPTIONS_H

#include <getopt.h>

typedef struct rmw_options rmw_options;

/** CLI option switches for rmw. */
struct rmw_options
{
  bool want_restore;
  bool want_purge;
  bool want_empty_trash;
  bool want_orphan_chk;
  bool want_selection_menu;
  bool want_undo;
  bool want_dry_run;
  int force;
  /*! list waste folder option */
  bool list;
  /*! Alternate configuration file given at the command line with -c */
  const char *alt_config;
};

void
init_rmw_options (rmw_options *options);

void
parse_cli_options (const int argc, char* const argv[], rmw_options *options);

#endif
