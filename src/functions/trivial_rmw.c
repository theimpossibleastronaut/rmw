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
  printf (_("\
Usage: rmw [OPTION]... FILE...\n\
ReMove the FILE(s) to a WASTE directory listed in configuration file\n\
\n\
   or: rmw -z FILE...\n\
Restore FILE(s) from a WASTE directory\n\
\n\
  -h, --help\n\
  -t, --translate           display a translation of the configuration file\n\
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
  -V, --version             display version and license information\n"));
  printf (_("\
  \n\n\
  \t===] Restoring [===\n\n\
  -z, --restore <wildcard filename(s) pattern>\n\
  -s, --select              select files from list to restore\n\
  -u, --undo-last           undo last ReMove\n"));
  printf (_("\
  \n\n\
  \t===] First time use [===\n\n\
After rmw is installed, create the user configuration directory\n\
by typing 'rmw' and hitting enter. It's recommended to copy\n\n\
  /etc/rmwrc (or /usr/local/etc/rmwrc) to '$HOME/.config/rmw'\n\n\
and then rename it to 'config':\n\n\
  'cd ~/.config/rmw'\n\
  '~/.config/rmw$ cp /etc/rmwrc .'\n\
  '~/.config/rmw$ mv rmwrc config'\n\n\
Then edit the file to suit your needs.\n\n\
Visit the rmw home page for more help, and information about\n\
how to obtain support - http://github.com/andy5995/rmw/wiki\n\n"));
  /* TRANSLATORS: The placeholder indicates the bug-reporting address
   for this package.  Please add _another line_ saying
   "Report translation bugs to <...>\n" with the address for translation
   bugs (typically your translation team's web or email address).  */
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
for details.\n"), VERSION);
  exit (0);
}

void
translate_config (void)
{
  /* TRANSLATORS:  Do not translate the last line in this section  */
  printf (_("\n\
# NOTE: If two WASTE folders are on the same file system, rmw will move files\n\
# to the first WASTE folder listed, ignoring the second one.\n\
#\n\
WASTE = $HOME/.trash.rmw\n"));

/* TRANSLATORS:  Do not translate the last line in this section  */
  printf (_("\n\
# If you would like this to be your primary trash folder (which usually means\n\
# that it will be the same as your Desktop Trash folder) be sure it precedes\n\
# any other WASTE folders listed in the config file\n\
#\n\
# If you want it checked for files that need purging, simply uncomment\n\
# The line below. Files you move with rmw will go to the folder above by\n\
# default.\n\
#\n\
#WASTE=$HOME/.local/share/Trash\n"));

/* TRANSLATORS:  Do not translate the last line in this section  */
  printf (_("\n\
# Removable media: If a folder has ',removable' appended to it, rmw\n\
# will not try to create it; it must be initially created manually. If\n\
# the folder exists when rmw is run, it will be used; if not, it will be\n\
# skipped Once you create \"example_waste\", rmw will automatically create\n\
# example_waste/info and example_waste/files\n\
#\n\
#WASTE=/mnt/sda10000/example_waste, removable"));

/* TRANSLATORS:  Do not translate the last line in this section  */
  printf (_("\n\
# How many days should files be allowed to stay in the waste folders before\n\
# they are permanently deleted\n\
#\n\
# use '0' to disable purging\n\
#\n\
purge_after = 90\n"));

/* TRANSLATORS:  Do not translate the last line in this section  */
  printf (_("\n\
# purge will not run unless `--force` is used at the command line. Uncomment\n\
# the line below if you would like purge to check daily for files that\n\
# that exceed the days specified in purge_after\n\
#\n\
#force_not_required\n"));

/* TRANSLATORS:  Do not translate the last two lines in this section  */
  printf (_("\n\
# If attempting to rmw files under this directory, they will not be processed\n\
# unless -B --bypass is given as an argument.\n\
#\n\
# PROTECT = /usr/local/bin\n\
# PROTECT = $HOME/src\n"));

}
