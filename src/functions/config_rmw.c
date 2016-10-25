/*
 * config_rmw.c
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

#include "config_rmw.h"

/**
 * Reads the config file, checks for the existence of waste directories,
 * and gets the value of 'purge_after'
 */
short
get_config_data(struct waste_containers *waste, const char *alt_config,
      unsigned short *purge_after,
      char protected_dir[PROTECT_MAX][MP], bool *force)
{
  char config_file[MP];
  const unsigned short CFG_MAX_LEN = PATH_MAX + 16;
  char line_from_config[CFG_MAX_LEN];

  /**
   *  purge_after will default to 90 if there's no setting
   * in the config file
   */
  //~ int default_purge = 90;
  *purge_after = 90;

  short func_error = 0;

  #ifndef WIN32
  char *HOMEDIR = getenv ("HOME");
  #else
  char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif

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

  if (bufchk (config_file, MP))
    return EXIT_BUF_ERR;

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
    strcpy (str_temp, SYSCONFDIR);
    strcat (str_temp, "/rmwrc");

    strcpy (config_file, str_temp);

    if (bufchk (config_file, MP))
      return EXIT_BUF_ERR;

    config_ptr = fopen (config_file, "r");

    if (config_ptr != NULL)
      config_opened = 1;
    else
    {
      open_err (config_file, __func__);
      fprintf (stderr, "\
Can not open configuration file\n\
%s (or)\n\
%s\n\
\n\
A default configuration file can be found at\n\
https://github.com/andy5995/rmw/tree/master/etc\n\
Terminating...\n", config_file, CFG_FILE);
      return 1;
    }
  }

  short waste_ctr = 0, prot_dir_ctr = 0;

  /**
   * protect DATA_DIR by default
   */
  strcpy (protected_dir[prot_dir_ctr], HOMEDIR);
  strcat (protected_dir[prot_dir_ctr], DATA_DIR);
  prot_dir_ctr++;

  while (fgets (line_from_config, CFG_MAX_LEN, config_ptr) != NULL)
  {
    if ((func_error = bufchk (line_from_config, CFG_MAX_LEN)))
    {
      fprintf (stderr, "Error: Lines in configuration file must be less than %d\n",
          CFG_MAX_LEN);
      break;
    }

    char *token_ptr;

    bool removable = 0;
    char *comma_ptr;

    trim (line_from_config);
    erase_char (' ', line_from_config);

    /**
     * assign purge_after the value from config file
     */
    if (strncmp (line_from_config, "purge_after", 11) == 0 ||
        strncmp (line_from_config, "purgeDays", 9) == 0)
    {
      token_ptr = strtok (line_from_config, "=");
      token_ptr = strtok (NULL, "=");

      erase_char (' ', token_ptr);

      ushort num_value = atoi (token_ptr);

      if (num_value >= 0 && num_value < USHRT_MAX)
        *purge_after = num_value;

      else
        fprintf (stderr, "Error: invalid purge_after value in configuration\n");
    }
    else if (strncmp (line_from_config, "force_not_required", 18) == 0)
      *force = 1;
    else if (waste_ctr < WASTENUM_MAX &&
        strncmp ("WASTE", line_from_config, 5) == 0)
    {
      token_ptr = strtok (line_from_config, "=");
      token_ptr = strtok (NULL, "=");
      char rem_opt[CFG_MAX_LEN];
      strcpy (rem_opt, token_ptr);

      comma_ptr = strtok (rem_opt, ",");
      comma_ptr = strtok (NULL, ",");

      /**
       * Does the line have the comma?
       */
      if (comma_ptr != NULL)
      {
        erase_char (' ', comma_ptr);
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
          fprintf (stderr, "Error: invalid option in config\n");
          continue;
        }
      }

      trim (token_ptr);
      trim_slash (token_ptr);
      erase_char (' ', token_ptr);
      make_home_real (token_ptr, HOMEDIR);

      strcpy (waste[waste_ctr].parent, token_ptr);
      strcpy (waste[waste_ctr].files, waste[waste_ctr].parent);

      strcat (waste[waste_ctr].files, "/files/");

      if ((func_error = bufchk (waste[waste_ctr].files, MP)))
        break;

      if (removable && exists (waste[waste_ctr].parent) != 0)
      {
        fprintf (stderr, "On unmounted device or not yet created: %s\n",
                  waste[waste_ctr].parent);
        continue;
      }

      if (make_dir (waste[waste_ctr].files))
        continue;

      strcpy (waste[waste_ctr].info, waste[waste_ctr].parent);
      strcat (waste[waste_ctr].info, "/info/");

      if (make_dir (waste[waste_ctr].info))
        continue;

      /**
       * protect WASTE dirs by default
       *
       * No need for a buffer check; they are declared the same as the
       * Waste folders
       */
      strcpy (protected_dir[prot_dir_ctr], waste[waste_ctr].parent);
      prot_dir_ctr++;

      /* get device number to use later for rename
       */
      struct stat st;
      lstat (waste[waste_ctr].parent, &st);
      waste[waste_ctr].dev_num = st.st_dev;

      waste_ctr++;
    }
    else if (prot_dir_ctr < PROTECT_MAX && !strncmp ("PROTECT", line_from_config, 7))
    {
      token_ptr = strtok (line_from_config, "=");
      token_ptr = strtok (NULL, "=");

      erase_char (' ', token_ptr);
      make_home_real (token_ptr, HOMEDIR);

      strcpy (protected_dir[prot_dir_ctr], token_ptr);

      if ((func_error = bufchk (protected_dir[prot_dir_ctr], MP)))
        break;

      prot_dir_ctr++;
    }

    if (waste_ctr == WASTENUM_MAX)
      fprintf (stdout, "Maximum number Waste folders reached: %d\n", WASTENUM_MAX);

    if (prot_dir_ctr == PROTECT_MAX)
      fprintf (stdout, "Maximum number protected folders reached: %d\n", PROTECT_MAX);
  }

  strcpy (waste[waste_ctr].parent, "NULL");
  strcpy (protected_dir[prot_dir_ctr], "NULL");
  /**
   * The earlier "breaks" will allow the config file to be closed here
   */
  if (config_opened)
  {
    if (!close_file (config_ptr, config_file, __func__))
      config_opened = 0;
  }

  if (waste_ctr == 0)
  {
    fprintf (stderr, "Error: no usable WASTE folder could be found\n");
    fprintf (stderr, "Please check your configuration file and permissions\n");
    fprintf (stderr, "If you need further help, or to report a possible bug, ");
    fprintf (stderr, "visit the rmw web site at\n");
    fprintf (stderr, "https://github.com/andy5995/rmw/wiki\n");
    fprintf (stderr, "Unable to continue. Exiting...\n");
    return NO_WASTE_FOLDER;
  }

  return func_error ? func_error : 0;
}

/**
 * if "$HOME" or "~" is used on configuration file
 * change to the value of "HOMEDIR"
 */

bool
make_home_real (char *str, const char *HOMEDIR)
{
  bool ok = 0;
  if (str[0] == '~')
  {
    erase_char ('~', str);
    ok = 1;
  }
  else if (strncmp (str, "$HOME", 5) == 0)
  {
    int chars_to_delete;

    for (chars_to_delete = 0; chars_to_delete < 5; chars_to_delete++)
      erase_char (str[0], str);

    ok = 1;
  }
  else
    return 0;

  char temp[MP];
  bufchk_string_op (COPY, temp, HOMEDIR, MP);
  bufchk_string_op (CONCAT, temp, str, MP);
  strcpy (str, temp);

  return ok;
}
