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

#include "parse_cli_options.h"
/* GIT_REV can defined during build-time with -D */
#ifdef GIT_REV
  #define RMW_VERSION_STRING VERSION "." GIT_REV
#else
  #define RMW_VERSION_STRING VERSION
#endif

static void
print_usage (void)
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
  -c, --config filename     use an alternate configuration\n\
  -l, --list                list waste directories\n\
  -g, --purge               run purge even if it's been run today\n\
  -o, --orphaned            check for orphaned files (maintenance)\n\
  -f, --force               allow rmw to purge files in the background\n\
  -e, --empty               completely empty (purge) all waste folders\n\
  -r, -R, --recursive       option used for compatibility with rm\n\
                            (recursive operation is enabled by default)\n\
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

static
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

static void
version (void)
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

void
init_rmw_options (rmw_options *options)
{
  verbose = 0;
  options->want_restore = false;
  options->want_dry_run = false;
  options->want_purge = false;
  options->want_empty_trash = false;
  options->want_orphan_chk = false;
  options->want_selection_menu = false;
  options->want_undo = false;
  options->force = 0;
  options->list = false;
  options->alt_config = NULL;
}


void
parse_cli_options (const int argc, char* const argv[], rmw_options *options)
{
  const char *const short_options = "hvc:goz:lnsuwVfeirR";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"dry-run", 0, NULL, 'n'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"orphaned", 0, NULL, 'o'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {"interactive", 0, NULL, 'i'},
    {"recursive", 0, NULL, 'r'},
    {"force", 0, NULL, 'f'},
    {"empty", 0, NULL, 'e'},
    {NULL, 0, NULL, 0}
  };

  int next_option = 0;

  do
  {
    next_option = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (next_option)
    {
    case 'h':
      print_usage ();
      exit (0);
    case 'v':
      verbose++;
      break;
    case 'c':
      options->alt_config = optarg;
      break;
    case 'n':
      options->want_dry_run = 1;
      puts (_("dry-run mode enabled."));
      /* assume verbosity as well */
      verbose = 1;
      break;
    case 'l':
      options->list = true;
      break;
    case 'g':
      options->want_purge = true;
      break;
    case 'o':
      options->want_orphan_chk = 1;
      break;
    case 'z':
      options->want_restore = true;
      break;
    case 's':
      options->want_selection_menu = 1;
      break;
    case 'u':
      options->want_undo = 1;
      break;
    case 'w':
      warranty ();
      break;
    case 'V':
      version ();
      break;
    case 'i':
      printf (_("-i / --interactive: not implemented\n"));
      break;
    case 'r':
    case 'R':
      printf (_("-r, -R, --recursive: option not required (enabled by default)\n"));
      break;
    case 'f':
      if (options->force < 2) /* This doesn't need to go higher than 2 */
        options->force++;
      break;
    case 'e':
      options->want_empty_trash = true;
      options->want_purge = true;
      break;
    case '?':
      print_usage ();
      exit (0);
    case -1:
      break;
    default:
      abort ();
    }

  }
  while (next_option != -1);

  return;
}
