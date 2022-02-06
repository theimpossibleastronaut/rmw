/*
This file is part of rmw<https://remove-to-waste.info/>

Copyright (C) 2012-2022  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <sys/stat.h>
#include <ctype.h>

#include "parse_cli_options.h"
#include "config_rmw.h"

#define RMW_VERSION_STRING VERSION


typedef enum
{
  HELP,
  VERBOSE,
  CONFIG,
  DRY_RUN,
  LIST,
  PURGE,
  ORPHANED,
  RESTORE,
  SELECT,
  UNDO_LAST,
  MOST_RECENT_LIST,
  WARRANTY,
  _VERSION,
  INTERACTIVE,
  RECURSIVE,
  FORCE,
  EMPTY,
} cli_opt_id;


static struct cli_opt
{
  cli_opt_id id;
  char *str;
} cli_opt[] = {
  {HELP, "help"},
  {VERBOSE, "verbose"},
  {CONFIG, "config"},
  {DRY_RUN, "dry-run"},
  {LIST, "list"},
  {PURGE, "purge"},
  {ORPHANED, "orphaned"},
  {RESTORE, "restore"},
  {SELECT, "select"},
  {UNDO_LAST, "undo-last"},
  {MOST_RECENT_LIST, "most-recent-list"},
  {WARRANTY, "warranty"},
  {_VERSION, "version"},
  {INTERACTIVE, "interactive"},
  {RECURSIVE, "recursive"},
  {FORCE, "force"},
  {EMPTY, "empty"}
};


/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum
{
  L_EMPTY = CHAR_MAX + 1,
};


static void
print_usage (const char *prog_name)
{
  printf (_("Usage: %s [OPTION]... FILE...\n\
Move FILE(s) to a WASTE directory listed in configuration file\n\
\n\
   or: %s -s\n\
   or: %s -u\n\
   or: %s -z FILE...\n\
"), prog_name, prog_name, prog_name, prog_name);
  puts (_("\
Restore FILE(s) from a WASTE directory"));
  putchar ('\n');
  printf (_("\
  -h, --%s                show help for command line options\n\
"), cli_opt[HELP].str);
  printf (_("\
  -c, --%s FILE         use an alternate configuration\n\
"), cli_opt[CONFIG].str);
  printf (_("\
  -l, --%s                list waste directories\n\
"), cli_opt[LIST].str);
  printf (_("\
  -g[N_DAYS], --%s[=N_DAYS]\n\
                            purge expired files;\n\
                            optional argument 'N_DAYS' overrides '%s'\n\
                            value from the configuration file\n\
                            (Examples: -g90, --purge=90)\n\
"), cli_opt[PURGE].str, expire_age_str);
  printf (_("\
  -o, --%s            check for orphaned files (maintenance)\n\
"), cli_opt[ORPHANED].str);
  printf (_("\
  -f, --%s               allow purging of expired files\n\
"), cli_opt[FORCE].str);
  printf (_("\
      --%s               completely empty (purge) all waste directories\n\
"), cli_opt[EMPTY].str);
  printf (_("\
  -r, -R, --%s       option used for compatibility with rm\n\
                            (recursive operation is enabled by default)\n\
"), cli_opt[RECURSIVE].str);
  printf (_("\
  -v, --%s             increase output messages\n\
"), cli_opt[VERBOSE].str);
  printf (_("\
  -w, --%s            display warranty\n\
"), cli_opt[WARRANTY].str);
  printf (_("\
  -V, --%s             display version and license information\n\
"), cli_opt[_VERSION].str);
  puts (_("\
\n\
\n\
  \t===] Restoring [===\n"));
  printf (_("\
  -z, --%s FILE(s)     restore FILE(s) (see man page example)\n\
"), cli_opt[RESTORE].str);
  printf (_("\
  -s, --%s              select files from list to restore\n\
"), cli_opt[SELECT].str);
  printf (_("\
  -u, --%s           undo last move\n\
"), cli_opt[UNDO_LAST].str);
  printf (_("\
  -m, --%s    list most recently rmw'ed files\n\
"), cli_opt[MOST_RECENT_LIST].str);
  puts (_("\
  \n\n\
Visit the rmw home page for more help, and information about\n\
how to obtain support - <" PACKAGE_URL ">\n"));
  printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);
}

static void
warranty (void)
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

/* Advise the user about invalid usages like "rm -foo" if the file
 * "-foo" exists, assuming ARGC and ARGV are as with 'main'.
 *
 * This function (lightly modified) is on loan from rm.c in the
 * GNU coreutils package<https://www.gnu.org/software/coreutils/coreutils.html> */
static void
diagnose_leading_hyphen (const int argc, char *const argv[])
{
  /* OPTIND is unreliable, so iterate through the arguments looking
     for a file name that looks like an option.  */
  int i;
  for (i = 1; i < argc; i++)
  {
    char const *arg = argv[i];
    struct stat st;

    if (arg[0] == '-' && arg[1] && lstat (arg, &st) == 0)
    {
      fprintf (stderr,
               _("Try '%s ./%s' to remove the file '%s'.\n"),
               argv[0], arg, arg);
      break;
    }
  }
}

void
init_rmw_options (rmw_options * x)
{
  verbose = 0;
  x->want_restore = false;
  x->want_dry_run = false;
  x->want_purge = 0;
  x->want_empty_trash = false;
  x->want_orphan_chk = false;
  x->want_selection_menu = false;
  x->want_undo = false;
  x->most_recent_list = false;
  x->force = 0;
  x->list = false;
  x->alt_config_file = NULL;
}


void
parse_cli_options (const int argc, char *const argv[], rmw_options * options)
{
  const struct option long_options[] = {
    {cli_opt[HELP].str, 0, NULL, 'h'},
    {cli_opt[VERBOSE].str, 0, NULL, 'v'},
    {cli_opt[CONFIG].str, 1, NULL, 'c'},
    {cli_opt[DRY_RUN].str, 0, NULL, 'n'},
    {cli_opt[LIST].str, 0, NULL, 'l'},
    {cli_opt[PURGE].str, 2, NULL, 'g'},
    {cli_opt[ORPHANED].str, 0, NULL, 'o'},
    {cli_opt[RESTORE].str, 1, NULL, 'z'},
    {cli_opt[SELECT].str, 0, NULL, 's'},
    {cli_opt[UNDO_LAST].str, 0, NULL, 'u'},
    {cli_opt[MOST_RECENT_LIST].str, 0, NULL, 'm'},
    {cli_opt[WARRANTY].str, 0, NULL, 'w'},
    {cli_opt[_VERSION].str, 0, NULL, 'V'},
    {cli_opt[INTERACTIVE].str, 0, NULL, 'i'},
    {cli_opt[RECURSIVE].str, 0, NULL, 'r'},
    {cli_opt[FORCE].str, 0, NULL, 'f'},
    {cli_opt[EMPTY].str, 0, NULL, L_EMPTY},
    {NULL, 0, NULL, 0}
  };

  int c;
  while ((c =
          getopt_long (argc, argv, "hvc:g::oz:lnsumwVfirR", long_options,
                       NULL)) != -1)
  {
    switch (c)
    {
    case 'h':
      print_usage (argv[0]);
      exit (0);
    case 'v':
      verbose++;
      break;
    case 'c':
      options->alt_config_file = optarg;
      break;
    case 'n':
      options->want_dry_run = true;
      puts (_("dry-run mode enabled."));
      /* assume verbosity as well */
      if (verbose == 0)
        verbose = 1;
      break;
    case 'l':
      options->list = true;
      break;
    case 'g':
      /* Ignore if used twice, but parse it if --purge was given no argument the first time */
      if (options->want_purge > 0)
        break;

      if (optarg == NULL)
      {
        options->want_purge = -1;
        break;
      }
      char *p = optarg;
      while (*p != '\0')
      {
        if (!isdigit (*p))
        {
          puts (_("Arguments given to --purge must be numeric only"));
          exit (EXIT_FAILURE);
        }
        p++;
      }
      options->want_purge = atoi (optarg);
      break;
    case 'o':
      options->want_orphan_chk = true;
      break;
    case 'z':
      options->want_restore = true;
      break;
    case 's':
      options->want_selection_menu = true;
      break;
    case 'u':
      options->want_undo = true;
      break;
    case 'm':
      options->most_recent_list = true;
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
      printf (_
              ("-r, -R, --recursive: option not required (enabled by default)\n"));
      break;
    case 'f':
      if (options->force < 2)   /* This doesn't need to go higher than 2 */
        options->force++;
      break;
    case L_EMPTY:
      options->want_empty_trash = true;
      options->want_purge = -1;
      break;
    default:
      diagnose_leading_hyphen (argc, argv);
      fprintf (stderr, _("Try '%s --help' for more information.\n"), argv[0]);
      exit (EXIT_FAILURE);
    }
  }

  return;
}
