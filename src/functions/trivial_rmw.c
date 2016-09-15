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
  printf ("-h, --help\n");
  printf ("-c, --config filename     use an alternate configuration\n");
  printf ("-l, --list                list waste directories\n");
  printf ("-p, --pause               wait for a keypress before exiting\n");
  printf
    ("-g, --purge               run purge even if it's been run today\n");
  printf ("-z, --restore <wildcard filename(s) pattern>\n");
  printf ("-f, --force               allow purge to run\n");
  printf ("-i, --interactive         not implemented\n");
  printf ("-r, --recurse             not implemented\n");
  printf ("-s, --select              select files from list to restore\n");
  printf ("-u, --undo-last           undo last ReMove\n");
  printf ("-B, --bypass              bypass directory protection\n");
  printf ("-v, --verbose             increase output messages\n");
  printf ("-w, --warranty            display warranty\n");
  printf
    ("-V, --version             display version and license information\n");
  printf("\nAfter rmw is installed, create the user configuration directory\n");
  printf("by typing 'rmw' and hitting enter. It's recommended to copy\n");
  printf("/etc/rmwrc (or /usr/local/etc/rmwrc) to '$HOME/.config/rmw'\n");
  printf("and then rename it to 'config':\n\n");
  printf("'cd ~/.config/rmw'\n");
  printf("'~/.config/rmw$ cp /etc/rmwrc .'\n");
  printf("'~/.config/rmw$ mv rmwrc config'\n\n");
  printf("Then edit the file to suit your needs.\n\n");
  printf("Visit the rmw home page for more help, and\n");
  printf("information about how to obtain support -\n");
  printf("http://github.com/andy5995/rmw/wiki\n\n");
}

void warranty (void)
{
  printf
    ("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n");
  printf
    ("APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n");
  printf
    ("HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\n");
  printf
    ("OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n");
  printf
    ("THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n");
  printf
    ("PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\n");
  printf
    ("IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\n");
  printf ("ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n");
  exit (0);
}

void version (void)
{

  printf ("rmw %s\n", VERSION);
  printf ("Author: Andy Alt (andy400-dev@yahoo.com)\n");
  printf
    ("This program comes with ABSOLUTELY NO WARRANTY; for details type 'rmw -w.'\n");
  printf ("This is free software, and you are welcome to redistribute it\n");
  printf
    ("under certain conditions; see <http://www.gnu.org/licenses/gpl.html>\n");
  printf ("for details.\n");
  exit (0);
}

void
err_display_check_config (void)
{
  fprintf (stderr, "Check your configuration file or permissions\n");
}

