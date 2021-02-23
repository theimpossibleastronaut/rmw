/*
 *
 * This file is part of rmw<https://remove-to-waste.info/>
 *
 *  Copyright (C) 2012-2021  Andy Alt (andy400-dev@yahoo.com)
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

#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H
#include "globals.h"
#endif

#include <sys/stat.h>
#include <ctype.h>

#include "parse_cli_options.h"
/* GIT_REV can defined during build-time with -D */
#ifdef GIT_REV
#define RMW_VERSION_STRING VERSION "." GIT_REV
#else
#define RMW_VERSION_STRING VERSION
#endif

static void
print_usage (const char *prog_name)
{
  printf (_("Usage: %s [OPTION]... FILE...\n\
ReMove the FILE(s) to a WASTE directory listed in configuration file\n\
\n\
   or: %s -s\n\
   or: %s -u\n\
   or: %s -z FILE...\n\
"), prog_name, prog_name, prog_name, prog_name);
  fputs (_("\
Restore FILE(s) from a WASTE directory\n\n\
"), stdout);
  fputs (_("\
  -h, --help                show help for command line options\n\
"), stdout);
  fputs (_("\
  -c, --config filename     use an alternate configuration\n\
"), stdout);
  fputs (_("\
  -l, --list                list waste directories\n\
"), stdout);
  fputs (_("\
  -g[N_DAYS], --purge[=N_DAYS]\n\
                            run purge even if it's been run today;\n\
                            optional argument 'N_DAYS' overrides 'purge_after'\n\
                            value from the configuration file\n\
                            (Examples: -g90, --purge=90)\n\
"), stdout);
  fputs (_("\
  -o, --orphaned            check for orphaned files (maintenance)\n\
"), stdout);
  fputs (_("\
  -f, --force               allow rmw to purge files in the background\n\
"), stdout);
  fputs (_("\
  -e, --empty               completely empty (purge) all waste folders\n\
"), stdout);
  fputs (_("\
  -r, -R, --recursive       option used for compatibility with rm\n\
                            (recursive operation is enabled by default)\n\
"), stdout);
  fputs (_("\
  -v, --verbose             increase output messages\n\
"), stdout);
  fputs (_("\
  -w, --warranty            display warranty\n\
"), stdout);
  fputs (_("\
  -V, --version             display version and license information\n\
"), stdout);
  fputs (_("\
  \n\n\
  \t===] Restoring [===\n\n"), stdout);
  fputs (_("\
  -z, --restore <wildcard filename(s) pattern> (e.g. ~/.local/share/Waste/files/foo*)\n\
"), stdout);
  fputs (_("\
  -s, --select              select files from list to restore\n\
"), stdout);
  fputs (_("\
  -u, --undo-last           undo last ReMove\n\
"), stdout);
  fputs (_("\
  -m, --most-recent-list    list most recently rmw'ed files\n\
"), stdout);
  fputs (_("\
  \n\n\
Visit the rmw home page for more help, and information about\n\
how to obtain support - \
"), stdout);
  printf ("<" PACKAGE_URL ">\n\n");
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
  for (int i = 1; i < argc; i++)
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
  x->alt_config = NULL;
}


void
parse_cli_options (const int argc, char *const argv[], rmw_options * options)
{
  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"dry-run", 0, NULL, 'n'},
    {"list", 0, NULL, 'l'},
    {"purge", 2, NULL, 'g'},
    {"orphaned", 0, NULL, 'o'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"most-recent-list", 0, NULL, 'm'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {"interactive", 0, NULL, 'i'},
    {"recursive", 0, NULL, 'r'},
    {"force", 0, NULL, 'f'},
    {"empty", 0, NULL, 'e'},
    {NULL, 0, NULL, 0}
  };

  int c;
  while ((c =
          getopt_long (argc, argv, "hvc:g::oz:lnsumwVfeirR", long_options,
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
      options->alt_config = optarg;
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
          printf ("Arguments given to --purge must be numeric only\n");
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
    case 'e':
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
