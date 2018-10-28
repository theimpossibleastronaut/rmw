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

/**
 * if "$HOME" or "~" is used on configuration file
 * change to the value of "HOMEDIR"
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

/**
 * Reads the config file, checks for the existence of waste directories,
 * and gets the value of 'purge_after'
 */
st_waste*
get_config_data(const char *alt_config, ushort *purge_after, const bool list,
  ushort *force, const char *HOMEDIR)
{
  char config_file[MP];
  const ushort CFG_MAX_LEN = PATH_MAX + 16;

  /**
   *  purge_after will default to 90 if there's no setting
   * in the config file
   */
   /* FIXME: A default value should be declared elsewhere
    */
  *purge_after = 90;

  /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
  {
    #ifndef WIN32
    strcpy (config_file, HOMEDIR);
    #else
    char *local_app_data = getenv ("LOCALAPPDATA");
    if (local_app_data != NULL)
      strcpy (config_file, local_app_data);
    #endif

    /**
     * CFG_FILE is the file name of the rmw config file relative to
     * the $HOME directory, defined at the top of rmw.h
     *
     * Create full path to config_file
     */
    strcat (config_file, CFG_FILE);

  }
  else
    strcpy (config_file, alt_config);

  bufchk (config_file, MP);

  FILE *config_ptr;

  /**
   * if config_file isn't found in home directory,
   * else open the system default (SYSCONFDIR/rmwrc)
   */

  bool config_opened = 0;

  if ((config_ptr = fopen (config_file, "r")) != NULL)
    config_opened = 1;

  if (config_opened)
  {}
  else
  {
    char str_temp[MP];
    sprintf (str_temp, "%s%s", SYSCONFDIR, "/rmwrc");

    strcpy (config_file, str_temp);
    bufchk (config_file, MP);

    config_ptr = fopen (config_file, "r");

    if (config_ptr != NULL)
      config_opened = 1;
    else
    {
      open_err (config_file, __func__);
       /* TRANSLATORS:  any time "open" or "close" is used in this program
        * I am referring to a file or a directory  */
      printf (_("\
  :Error: Can not open configuration file\n\
%s (or)\n\
%s%s\n\
\n\
A default configuration file can be found at\n\
https://github.com/andy5995/rmw/tree/master\n\
Terminating...\n"), config_file, HOMEDIR, CFG_FILE);
      msg_return_code (ERR_OPEN_CONFIG);
      exit (ERR_OPEN_CONFIG);
    }
  }

  st_waste *waste_head = NULL;
  st_waste *waste_curr = NULL;

  char *line_from_config = calloc (CFG_MAX_LEN + 1, 1);

  while (fgets (line_from_config, CFG_MAX_LEN, config_ptr) != NULL)
  {
    bufchk (line_from_config, CFG_MAX_LEN);

    char *token_ptr;

    bool removable = 0;
    char *comma_ptr;

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
      token_ptr = strtok (line_from_config, "=");
      token_ptr = strtok (NULL, "=");
      char rem_opt[CFG_MAX_LEN];
      bufchk (token_ptr, CFG_MAX_LEN);
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
          continue;
        }
      }

      trim (token_ptr);
      trim_slash (token_ptr);
      del_char_shift_left (' ', &token_ptr);
      make_home_real (&token_ptr, HOMEDIR);

      if (removable && exists (token_ptr) != 0)
      {
        if (list)
          printf (_("%s (removable, detached)\n"), token_ptr);
        continue;
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
        waste_head = temp_node;
        waste_curr = waste_head;
        waste_curr->prev_node = NULL;
      }

      /* make the parent... */
      strcpy (waste_curr->parent, token_ptr);

      /* and the files... */
      sprintf (waste_curr->files, "%s%s", waste_curr->parent, "/files/");
      bufchk (waste_curr->files, MP);

      if (exists (waste_curr->files))
      {
        if (make_dir (waste_curr->files) == MAKE_DIR_FAILURE)
        {
          exit (EXIT_FAILURE);
        }
      }

        /* and the info. */
      sprintf (waste_curr->info, "%s%s", waste_curr->parent, "/info/");

      if (exists (waste_curr->info))
      {
        if (make_dir (waste_curr->info) == MAKE_DIR_FAILURE)
        {
          exit (EXIT_FAILURE);
        }
      }

      /* get device number to use later for rename
       */
      struct stat st;
      lstat (waste_curr->parent, &st);
      waste_curr->dev_num = st.st_dev;
    }
    else if (!strncmp ("PROTECT", line_from_config, 7))
    {
      static bool pctr = 0;
      if (!pctr)
        printf ("The PROTECT feature has been removed.\n");
      pctr = 1;
    }

  }
  free (line_from_config);

  /**
   * The earlier "breaks" will allow the config file to be closed here
   */
  if (config_opened)
  {
    if (!close_file (config_ptr, config_file, __func__))
      config_opened = 0;
  }

  if (waste_curr == NULL)
  {
    printf (_("  :Error: no usable WASTE folder could be found\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n\
  https://github.com/andy5995/rmw/wiki\n\
Unable to continue. Exiting...\n"));
    msg_return_code (NO_WASTE_FOLDER);
    exit (NO_WASTE_FOLDER);
  }
  else
   waste_curr->next_node = NULL;

  return waste_head;
}
