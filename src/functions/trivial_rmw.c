/*
 * trivial_rmw.c
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

#include "trivial_rmw.h"

void print_usage (void)
{
  printf ("\
Usage: rmw [OPTION]... FILE...\n\
ReMove the FILE(s) to a WASTE directory listed in configuration file\n\
\n\
   or: rmw -z FILE...\n\
Restore FILE(s) from a WASTE directory\n\
\n\
  -h, --help\n\
  -c, --config filename     use an alternate configuration\n\
  -l, --list                list waste directories\n\
  -g, --purge               run purge even if it's been run today\n\
  -o, --orphaned            check for orphaned files (maintenance)\n\
  -f, --force               allow purge to run\n\
  -i, --interactive         not implemented\n\
  -r, --recurse             not implemented\n\
  -B, --bypass              bypass directory protection\n\
  -v, --verbose             increase output messages\n\
  -w, --warranty            display warranty\n\
  -V, --version             display version and license information\n");
  printf ("\
  \n\n\
  \t===] Restoring [===\n\n\
  -z, --restore <wildcard filename(s) pattern>\n\
  -s, --select              select files from list to restore\n\
  -u, --undo-last           undo last ReMove\n");
  printf ("\
  \n\n\
  t===] First time use [===\n\n\
After rmw is installed, create the user configuration directory\n\
by typing 'rmw' and hitting enter. It's recommended to copy\n\n\
  /etc/rmwrc (or /usr/local/etc/rmwrc) to '$HOME/.config/rmw'\n\n\
and then rename it to 'config':\n\n\
  'cd ~/.config/rmw'\n\
  '~/.config/rmw$ cp /etc/rmwrc .'\n\
  '~/.config/rmw$ mv rmwrc config'\n\n\
Then edit the file to suit your needs.\n\n\
Visit the rmw home page for more help, and information about\n\
how to obtain support - http://github.com/andy5995/rmw/wiki\n\n");
}

void warranty (void)
{
printf ("\
THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n\
APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n\
HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\n\
OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n\
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n\
PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\n\
IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\n\
ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n");

  exit (0);
}

void version (void)
{
  printf ("\
rmw %s\n\
Author: Andy Alt (andy400-dev@yahoo.com)\n\
The RMW team: see AUTHORS file\n\
This program comes with ABSOLUTELY NO WARRANTY; for details type 'rmw -w.'\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see <http://www.gnu.org/licenses/gpl.html>\n\
for details.\n", VERSION);
  exit (0);
}
