/*
 * config_rmw.c
 *
 * This file is part of rmw (http://rmw.sf.net)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andyqwerty@users.sourceforge.net)
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
 * Check for the existence of the data dir, and create it if not found
 */
void
check_for_data_dir (const char *data_dir)
{
  if (file_not_found (data_dir) == 0)
    return;

  char temp_data_dir[MP];
  strcpy (temp_data_dir, data_dir);

  char *tokenPtr;
  tokenPtr = strtok (temp_data_dir, "/");

  char add_to_path[MP];
  strcat (add_to_path, "/\0");

  while (tokenPtr != NULL)
  {
    strcat (add_to_path, tokenPtr);
    strcat (add_to_path, "/");
    tokenPtr = strtok (NULL, "/");

    if (file_not_found (add_to_path))
      if (!mkdir (add_to_path, S_IRWXU))
        fprintf (stdout, "Created %s\n", add_to_path);

      else
      {
        fprintf (stderr, "Error %d while creating %s\n", errno, temp_data_dir);
        err_display_check_config ();
      }
  }
}

/**
 * Reads the config file, checks for the existence of waste directories,
 * and gets the value of 'purge_after'
 */
int
get_config_data(struct waste_containers *waste, const char *alt_config,
                const char *HOMEDIR, int *pa, bool list, int *waste_ctr,
                char pro_dir[PROTECT_MAX][MP], int *pro_ctr)
{
  char config_file[MP];
  const unsigned short CFG_MAX_LEN = PATH_MAX + 16;
  char confLine[CFG_MAX_LEN];

  /**
   *  purge_after will default to 90 if there's no setting
   * in the config file
   */
  int default_purge = 90;
  *pa = default_purge;

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
  if (!file_not_found (config_file))
  {
    cfgPtr = fopen (config_file, "r");

    /**
     * It exists. Any problem opening it for reading?
     */
    if (cfgPtr == NULL)
    {
      perror ("get_config_data");
      fprintf (stderr, "Error %d reading %s\n", errno, config_file);
    }
  }

  else
  {
    buf_check_with_strop (config_file, SYSCONFDIR "/rmwrc", CPY);

    if (!file_not_found (config_file))
      cfgPtr = fopen (config_file, "r");

    if (errno)
    {
      fprintf (stderr, "Error %d reading %s\n", errno, config_file);
      fprintf (stderr, "Can not open configuration file\n");
      fprintf (stderr, "%s (or)\n", config_file);
      fprintf (stderr, "%s\n", CFG_FILE);
      fprintf (stderr, "\nA default configuration file can be found at\n");
      fprintf (stderr, "https://github.com/andy5995/rmw/tree/master/etc\n");
      fprintf (stderr, "Terminating...\n");
      return 1;
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
      *pa = atoi (tokenPtr);
    }
    else if (*waste_ctr < WASTENUM_MAX && !strncmp ("WASTE", confLine, 5))
    {

      tokenPtr = strtok (confLine, "=");
      tokenPtr = strtok (NULL, "=");

      trim_slash (tokenPtr);
      erase_char (' ', tokenPtr);
      change_HOME (tokenPtr, HOMEDIR);

      buf_check_with_strop (waste[*waste_ctr].parent, tokenPtr, CPY);

      // Make WASTE/files string
      /* No need to check boundaries for the copy */
      strcpy (waste[*waste_ctr].files, waste[*waste_ctr].parent);
      buf_check_with_strop (waste[*waste_ctr].files, "/files/", CAT);

      // Make WASTE/info string
      /* No need to check boundaries for the copy */
      strcpy (waste[*waste_ctr].info, waste[*waste_ctr].parent);
      buf_check_with_strop (waste[*waste_ctr].info, "/info/", CAT);

      // Create WASTE if it doesn't exit
      waste_check (waste[*waste_ctr].parent);

      /**
       * protect WASTE dirs by default
       */
      strcpy (pro_dir[*pro_ctr], waste[*waste_ctr].parent);
      (*pro_ctr)++;

      // Create WASTE/files if it doesn't exit
      waste_check (waste[*waste_ctr].files);

      // Create WASTE/info if it doesn't exit
      waste_check (waste[*waste_ctr].info);

      // get device number to use later for rename
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
      change_HOME (tokenPtr, HOMEDIR);

      buf_check (tokenPtr, MP);

      realpath (tokenPtr, pro_dir[*pro_ctr]);

      buf_check (pro_dir[*pro_ctr], MP);

      (*pro_ctr)++;
    }

    if (*waste_ctr == WASTENUM_MAX)
      fprintf (stdout, "Maximum number Waste folders reached: %d\n", WASTENUM_MAX);

    if (*pro_ctr == PROTECT_MAX)
      fprintf (stdout, "Maximum number protected folders reached: %d\n", PROTECT_MAX);
  }
  if (fclose (cfgPtr) == EOF)
    fprintf (stderr, "Error %d closing %s\n", errno, config_file);

  // No WASTE= line found in config file
  if (*waste_ctr == 0)
  {
    fprintf (stderr, "No 'WASTE=' line config file (%s) error; exiting...\n",
             config_file);
    exit (1);
  }
}
