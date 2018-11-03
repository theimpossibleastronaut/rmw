/*
 * config_rmw.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2018  Andy Alt (andy400-dev@yahoo.com)
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

#include "config_rmw.h"
#include "utils_rmw.h"
#include "strings_rmw.h"
#include "trivial_rmw.h"
#include "messages_rmw.h"

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
static void del_char_shift_left (const char c, char **str)
{
  while (**str == c)
    ++(*str);

  return;
}

/*
 *
 * name: make_home_real
 *
 * if "$HOME" or "~" is used on configuration file
 * change to the value of "HOMEDIR"
 *
 */
static bool make_home_real (char **str, const char *HOMEDIR)
{
  bool ok = 0;
  if (*str[0] == '~')
  {
    del_char_shift_left ('~', str);
    ok = 1;
  }
  else if (strncmp (*str, "$HOME", 5) == 0)
  {
    int chars_to_delete;

    for (chars_to_delete = 0; chars_to_delete < 5; chars_to_delete++)
      del_char_shift_left (*str[0], str);

    ok = 1;
  }
  else
    return 0;

  char temp[MP];
  sprintf (temp, "%s%s", HOMEDIR, *str);
  strcpy (*str, temp);

  return ok;
}

/*
 *
 * name: parse_line_waste
 *
 * This function is called when the "WASTE" option is encountered in the
 * config file.
 *
 */
static st_waste*
parse_line_waste(char *token_ptr, st_waste *waste_curr, char *line_from_config,
  const char *HOMEDIR, bool *do_continue)
{
  bool removable = 0;
  char *comma_ptr;
  token_ptr = strtok (line_from_config, "=");
  token_ptr = strtok (NULL, "=");
  char rem_opt[CFG_LINE_LEN_MAX];
  bufchk (token_ptr, CFG_LINE_LEN_MAX);
  strcpy (rem_opt, token_ptr);

  comma_ptr = strtok (rem_opt, ",");
  comma_ptr = strtok (NULL, ",");

  /**
   * Does the line have the comma?
   */
  if (comma_ptr != NULL)
  {
    del_char_shift_left (' ', &comma_ptr);
    unsigned int ctr = 0;

    while (ctr < strlen (token_ptr))
    {
      if (token_ptr[ctr] == ',')
        token_ptr[ctr] = '\0';

      ctr++;
    }

    if (strcmp ("removable", comma_ptr) == 0)
      removable = 1;
    else
    {
      printf (_("  :Error: invalid option in config\n"));
      goto DO_CONT;
    }
  }

  trim (token_ptr);
  trim_slash (token_ptr);
  del_char_shift_left (' ', &token_ptr);
  make_home_real (&token_ptr, HOMEDIR);

  if (removable && !exists (token_ptr))
  {
    if (list)
      printf (_("%s (removable, detached)\n"), token_ptr);
    goto DO_CONT;
  }

  st_waste *temp_node = (st_waste*)malloc (sizeof (st_waste));
  chk_malloc (temp_node , __func__, __LINE__);

  if (waste_curr != NULL)
  {
    waste_curr->next_node = temp_node;
    temp_node->prev_node = waste_curr;
    waste_curr = waste_curr->next_node;
  }
  else
  {
    waste_curr = temp_node;
    waste_curr->prev_node = NULL;
  }

  waste_curr->removable = removable ? true : false;

  /* make the parent... */
  strcpy (waste_curr->parent, token_ptr);

  /* and the files... */
  sprintf (waste_curr->files, "%s%s", waste_curr->parent, "/files/");
  bufchk (waste_curr->files, MP);

  if (!exists (waste_curr->files))
  {
    if (make_dir (waste_curr->files) == MAKE_DIR_FAILURE)
    {
      exit (EXIT_FAILURE);
    }
  }

    /* and the info. */
  sprintf (waste_curr->info, "%s%s", waste_curr->parent, "/info/");

  if (!exists (waste_curr->info))
  {
    if (make_dir (waste_curr->info) == MAKE_DIR_FAILURE)
    {
      exit (EXIT_FAILURE);
    }
  }

  /* get device number to use later for rename
   *
   * coverity complains if the return value of lstat isn't checked.
   * Really, if we get to this point, lstat shouldn't have any problem,
   * checking return values is good practice so we'll do it.
   */
  struct stat st;
  if (lstat (waste_curr->parent, &st) == 0)
    waste_curr->dev_num = st.st_dev;
  else
  {
    printf (MSG_WARNING);
    perror ("lstat()");
  }

  return waste_curr;

  DO_CONT:
    *do_continue = 1;
    return waste_curr;
}

/*
 *
 * name: realize_config_file
 *
 * determine the exact name of the config file to use, and return a file pointer
 *
 */
static FILE*
realize_config_file (const char *alt_config, char *config_file, const char *HOMEDIR)
{
 /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
  {
    /**
     * CFG_FILE is the file name of the rmw config file relative to
     * the $HOME directory, defined at the top of rmw.h
     *
     * Create full path to config_file
     */
    bufchk (CFG_FILE, MP - strlen (HOMEDIR));
    sprintf (config_file, "%s%s", HOMEDIR, CFG_FILE);
  }
  else
  {
    bufchk (alt_config, MP);
    strcpy (config_file, alt_config);
  }

  /**
   * if config_file isn't found in home directory,
   * else open the system default (SYSCONFDIR/rmwrc)
   */

  FILE *fp;

  fp = fopen (config_file, "r");
  if (fp != NULL)
    return fp;

  bufchk ("/rmwrc", MP - strlen (SYSCONFDIR));
  sprintf (config_file, "%s%s", SYSCONFDIR, "/rmwrc");

  fp = fopen (config_file, "r");
  if (fp != NULL)
    return fp;
  else
  {
    open_err (config_file, __func__);
     /* TRANSLATORS:  any time "open" or "close" is used in this program
      * I am referring to a file or a directory  */
    printf (MSG_ERROR);
    printf (_("\
Can not open configuration file\n\
%s (or)\n\
%s%s\n\
\n\
A default configuration file can be found at\n"), config_file, HOMEDIR, CFG_FILE);
    printf ("<https://github.com/theimpossibleastronaut/rmw/tree/master>\n\n");
    printf (_("Unable to continue. Exiting...\n"));
    msg_return_code (ERR_OPEN_CONFIG);
    exit (ERR_OPEN_CONFIG);
  }
}

/*
 *
 * name: get_config_data
 *
 * get configuration data (parse the config file), returns a pointer to
 * a linked list, each node containing info about usable "waste"
 * directories
 *
 */
st_waste*
get_config_data(const char *alt_config, ushort *purge_after,
  ushort *force, const char *HOMEDIR)
{
  /**
   *  purge_after will default to 90 if there's no setting
   * in the config file
   */
   /* FIXME: A default value should be declared elsewhere
    */
  *purge_after = 90;

  char config_file[MP];
  FILE *config_ptr = realize_config_file (alt_config, config_file, HOMEDIR);

  st_waste *waste_head = NULL;
  st_waste *waste_curr = NULL;
  char *line_from_config = calloc (CFG_LINE_LEN_MAX + 1, 1);
  while (fgets (line_from_config, CFG_LINE_LEN_MAX, config_ptr) != NULL)
  {
    bool do_continue = 0;
    bufchk (line_from_config, CFG_LINE_LEN_MAX);

    char *token_ptr;

    trim (line_from_config);
    del_char_shift_left (' ', &line_from_config);

    /**
     * assign purge_after the value from config file
     */
    if (strncmp (line_from_config, "purge_after", 11) == 0 ||
        strncmp (line_from_config, "purgeDays", 9) == 0)
    {
      token_ptr = strtok (line_from_config, "=");
      token_ptr = strtok (NULL, "=");

      del_char_shift_left (' ', &token_ptr);

      ushort num_value = atoi (token_ptr);

      if (num_value >= 0 && num_value < USHRT_MAX)
        *purge_after = num_value;

      else
        /* TRANSLATORS:  "purge_after" is a varible  */
        printf (_("  :Error: invalid purge_after value in configuration\n"));
    }
    else if (!*force && strncmp (line_from_config, "force_not_required", 18) == 0)
      {
        *force = 1;
      }
    else if (strncmp ("WASTE", line_from_config, 5) == 0)
    {
      waste_curr = parse_line_waste (token_ptr, waste_curr, line_from_config, HOMEDIR, &do_continue);
      if (do_continue)
        continue;
    }
    else if (!strncmp ("PROTECT", line_from_config, 7))
    {
      static bool pctr = 0;
      if (!pctr)
        printf ("The PROTECT feature has been removed.\n");
      pctr = 1;
    }
    if (waste_curr != NULL && waste_head == NULL)
        waste_head = waste_curr;
  }
  line_from_config = NULL;
  free (line_from_config);

  close_file (config_ptr, config_file, __func__);

  if (waste_curr == NULL)
  {
    printf (_("  :Error: no usable WASTE folder could be found\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n"));
    printf ("  " PACKAGE_URL "\n");
    printf ("Unable to continue. Exiting...\n");
    msg_return_code (NO_WASTE_FOLDER);
    exit (NO_WASTE_FOLDER);
  }
  else
   waste_curr->next_node = NULL;

  return waste_head;
}
