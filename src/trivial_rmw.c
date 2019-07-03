/*!
 * @file trivial_rmw.c
 */
/*
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

#ifndef INC_RMW_H
#define INC_RMW_H
  #include "rmw.h"
#endif

#include "trivial_rmw.h"
#include "git_rev.h"
/* GIT_VER is defined during build-time with -D, see src/Makefile.am */
#ifdef GIT_REV
  #define RMW_VERSION_STRING VERSION "." GIT_REV
#else
  #define RMW_VERSION_STRING VERSION
#endif

void print_usage (void)
{
  printf (_("\
Usage: rmw [OPTION]... FILE...\n\
ReMove the FILE(s) to a WASTE directory listed in configuration file\n\
\n\
   or: rmw -s\n\
   or: rmw -u\n\
   or: rmw -z FILE...\n\
Restore FILE(s) from a WASTE directory\n\
\n\
  -h, --help\n\
  -t, --translate           display a translation of the configuration file\n\
  -c, --config filename     use an alternate configuration\n\
  -l, --list                list waste directories\n\
  -g, --purge               run purge even if it's been run today\n\
  -o, --orphaned            check for orphaned files (maintenance)\n\
  -f, --force               allow rmw to purge files in the background\n\
  -e, --empty               completely empty (purge) all waste folders\n\
  -v, --verbose             increase output messages\n\
  -w, --warranty            display warranty\n\
  -V, --version             display version and license information\n"));
  printf (_("\
  \n\n\
  \t===] Restoring [===\n\n\
  -z, --restore <wildcard filename(s) pattern>\n\
  -s, --select              select files from list to restore\n\
  -u, --undo-last           undo last ReMove\n"));
  printf (_("\
  \n\n\
Visit the rmw home page for more help, and information about\n\
how to obtain support - "));
  printf ("<" PACKAGE_URL ">\n\n");
  printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);
}

void warranty (void)
{
printf (_("\
THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n\
APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n\
HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\n\
OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n\
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n\
PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\n\
IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\n\
ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n"));

  exit (0);
}

void version (void)
{
  printf (_("\
rmw %s\n\
Author: Andy Alt (andy400-dev@yahoo.com)\n\
The RMW team: see AUTHORS file\n\
This program comes with ABSOLUTELY NO WARRANTY; for details type 'rmw -w.'\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see <http://www.gnu.org/licenses/gpl.html>\n\
for details.\n"), RMW_VERSION_STRING);
  exit (0);
}
