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
                const char *HOMEDIR, unsigned short *purge_after_ptr, bool list, int *waste_ctr,
                char pro_dir[PROTECT_MAX][MP], int *pro_ctr, bool *force_ptr)
{
  char config_file[MP];
  const unsigned short CFG_MAX_LEN = PATH_MAX + 16;
  char confLine[CFG_MAX_LEN];

  /**
   *  purge_after will default to 90 if there's no setting
   * in the config file
   */
  int default_purge = 90;
  *purge_after_ptr = default_purge;

  /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
  {
    /**
     * Besides boundary checking, buf_check_with_strop()
     * will perform the strcpy or strcat)
     */

    buf_check_with_strop (config_file, HOMEDIR, CPY);

    /**
     * CFG_FILE is the file name of the rmw config file relative to
     * the $HOME directory, defined at the top of rmw.h
     *
     * Create full path to config_file
     */
    buf_check_with_strop (config_file, CFG_FILE, CAT);
  }
  else
    buf_check_with_strop (config_file, alt_config, CPY);

  FILE *cfgPtr;

  /**
   * if config_file isn't found in home directory,
   * else open the system default (SYSCONFDIR/rmwrc)
   */

  bool config_opened = 0;

  if (!file_not_found (config_file))
  {
    if ((cfgPtr = fopen (config_file, "r")) != NULL)
      config_opened = 1;
    else
      open_err (config_file, __func__);
  }

  if (config_opened)
  {}

  else
  {
    char str_temp[MP];
    strcpy (str_temp, SYSCONFDIR);
    strcat (str_temp, "/rmwrc");
    buf_check_with_strop (config_file, str_temp, CPY);

    if (!file_not_found (config_file))
    {
      cfgPtr = fopen (config_file, "r");

      if (cfgPtr != NULL)
        config_opened = 1;

      else
      {
        fprintf (stderr, "Error: reading %s\n", config_file);
        perror (__func__);
        fprintf (stderr, "Can not open configuration file\n");
        fprintf (stderr, "%s (or)\n", config_file);
        fprintf (stderr, "%s\n", CFG_FILE);
        fprintf (stderr, "\nA default configuration file can be found at\n");
        fprintf (stderr, "https://github.com/andy5995/rmw/tree/master/etc\n");
        fprintf (stderr, "Terminating...\n");
        return 1;
      }
    }
  }

  *waste_ctr = 0;
  *pro_ctr = 0;

  /**
   * protect DATA_DIR by default
   */
  strcpy (pro_dir[*pro_ctr], HOMEDIR);
  strcat (pro_dir[*pro_ctr], DATA_DIR);
  (*pro_ctr)++;

  while (fgets (confLine, CFG_MAX_LEN, cfgPtr) != NULL)
  {
    char *tokenPtr;

    bool removable = 0;
    char *comma_ptr;

    buf_check (confLine, CFG_MAX_LEN);
    trim (confLine);
    erase_char (' ', confLine);

    /**
     * assign purge_after the value from config file
     */
    if (strncmp (confLine, "purge_after", 11) == 0 || strncmp(confLine, "purgeDays", 9) == 0)
    {
      tokenPtr = strtok (confLine, "=");
      tokenPtr = strtok (NULL, "=");
      // buf_check (tokenPtr, 4096);
      erase_char (' ', tokenPtr);
      buf_check (tokenPtr, 6);
      *purge_after_ptr = atoi (tokenPtr);
    }

    else if (strncmp (confLine, "force_not_required", 18) == 0)
      *force_ptr = 1;

    else if (*waste_ctr < WASTENUM_MAX && !strncmp ("WASTE", confLine, 5))
    {
      tokenPtr = strtok (confLine, "=");
      tokenPtr = strtok (NULL, "=");

      char rem_opt[CFG_MAX_LEN];
      strcpy (rem_opt, tokenPtr);

      comma_ptr = strtok (rem_opt, ",");
      comma_ptr = strtok (NULL, ",");

      /**
       * Does the line have the comma?
       */
      if (comma_ptr != NULL)
      {
        erase_char (' ', comma_ptr);
        unsigned int ctr = 0;

        while (ctr < strlen (tokenPtr))
        {
          if (tokenPtr[ctr] == ',')
            tokenPtr[ctr] = '\0';

          ctr++;
        }

        if (strcmp ("removable", comma_ptr) == 0)
          removable = 1;
        else
          fprintf (stderr, "invalid option in config\n");
      }

      trim (tokenPtr);
      trim_slash (tokenPtr);
      erase_char (' ', tokenPtr);
      make_home_real (tokenPtr, HOMEDIR);

      buf_check_with_strop (waste[*waste_ctr].parent, tokenPtr, CPY);

      strcpy (waste[*waste_ctr].files, waste[*waste_ctr].parent);

      buf_check_with_strop (waste[*waste_ctr].files, "/files/", CAT);

      if (removable && file_not_found (waste[*waste_ctr].parent))
      {
        fprintf (stderr, "On unmounted device or not yet created: %s\n",
                  waste[*waste_ctr].parent);
        continue;
      }

      if (make_dir (waste[*waste_ctr].files))
        continue;


      strcpy (waste[*waste_ctr].info, waste[*waste_ctr].parent);
      buf_check_with_strop (waste[*waste_ctr].info, "/info/", CAT);
      if (make_dir (waste[*waste_ctr].info))
        continue;

      /**
       * protect WASTE dirs by default
       */
      strcpy (pro_dir[*pro_ctr], waste[*waste_ctr].parent);
      (*pro_ctr)++;

      /* get device number to use later for rename
       */
      struct stat st;
      lstat (waste[*waste_ctr].parent, &st);
      waste[*waste_ctr].dev_num = st.st_dev;

      if (list)
        printf ("%s\n", waste[*waste_ctr].parent);

      (*waste_ctr)++;
    }

    else if (*pro_ctr < PROTECT_MAX && !strncmp ("PROTECT", confLine, 7))
    {
      tokenPtr = strtok (confLine, "=");
      tokenPtr = strtok (NULL, "=");

      buf_check (tokenPtr, MP);
      erase_char (' ', tokenPtr);
      make_home_real (tokenPtr, HOMEDIR);

      buf_check (tokenPtr, MP);

      resolve_path (tokenPtr, pro_dir[*pro_ctr]);

      buf_check (pro_dir[*pro_ctr], MP);

      (*pro_ctr)++;
    }

    if (*waste_ctr == WASTENUM_MAX)
      fprintf (stdout, "Maximum number Waste folders reached: %d\n", WASTENUM_MAX);

    if (*pro_ctr == PROTECT_MAX)
      fprintf (stdout, "Maximum number protected folders reached: %d\n", PROTECT_MAX);
  }

  if (config_opened)
  {
    if (!close_file (cfgPtr, config_file, __func__))
      config_opened = 0;
  }

  if (*waste_ctr == 0)
  {
    fprintf (stderr, "Error: no usable WASTE folder could be found\n");
    fprintf (stderr, "Please check your configuration file and permissions\n");
    fprintf (stderr, "If you need further help, or to report a possible bug, ");
    fprintf (stderr, "visit the rmw web site at\n");
    fprintf (stderr, "https://github.com/andy5995/rmw/wiki\n");
    fprintf (stderr, "Unable to continue. Exiting...\n");
    return NO_WASTE_FOLDER;
  }

  return 0;
}
