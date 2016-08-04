/*
 * primary_funcs.c
 *
 * This file is part of rmw (https://rmw.sf.net)
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

#include "rmw.h"
#include "function_prototypes.h"


/* Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 * */

bool buf_check_with_strop (char *s1, const char *s2, bool mode)
{
  unsigned int len1, len2;
  len1 = strlen (s1);
  len2 = strlen (s2);
  if (len2 < MP && mode == CPY)
    {
      strcpy (s1, s2);
      return 0;
    }
  else if (len1 + len2 < MP && mode == CAT)
    {
      strcat (s1, s2);
      return 0;
    }

  else if (mode > CAT || mode < CPY)
    {
      printf
	("error in function: buf_check_with_strop() - mode must be either CPY (0) or CAT (1)");
      abort ();
    }

  else
    {
      printf ("Potential buffer overflow caught. rmw terminating.\n");
      exit (-1);
      return 1;
    }

}

bool buf_check (const char *str, unsigned short boundary)
{

  unsigned short len = strlen (str);

  if (len >= boundary)
    {
      printf ("Potential buffer overrun caught; rmw terminating.\n");
      exit (1);
    }

  return 0;
}

int get_config (const char *alt_config, int purge_after)
{

  struct stat st;
  FILE *cfgPtr;
  wasteNum = 0;
  char *tokenPtr;
  char configFile[MP];
  const unsigned short CFG_MAX_LEN = PATH_MAX + 16;
  char confLine[CFG_MAX_LEN];

  /* If no alternate configuration was specifed (-c) */
  if (alt_config == NULL)
    {
	/**
	 * Besides boundary checking, buf_check_with_strop()
	 * will perform the strcpy or strcat)
	 */

      /* copy $HOMEDIR to configFile */
      buf_check_with_strop (configFile, HOMEDIR, CPY);

      // CFG is the file name of the rmw config file relative to
      // the $HOME directory, defined at the top of rmw.c
      /* create full path to configFile */
      buf_check_with_strop (configFile, CFG, CAT);
    }
  else
    buf_check_with_strop (configFile, alt_config, CPY);

  cfgPtr = fopen (configFile, "r");
  /* if configFile isn't found in home directory,
   *  open the system default */
  if (cfgPtr == NULL)
    {
      /* buf_check_with_strop(configFile, "/etc/rmwrc", CPY); */
      buf_check_with_strop (configFile, SYSCONFDIR "/rmwrc", CPY);
      cfgPtr = fopen (configFile, "r");
      if (cfgPtr == NULL)
	{
	  fprintf (stderr, "Configuration file (%s) not found.\nExiting...\n",
		   configFile);
	  exit (1);
	}
    }

  while (fgets (confLine, CFG_MAX_LEN, cfgPtr) != NULL)
    {
      buf_check (confLine, CFG_MAX_LEN);
      trim (confLine);
      erase_char (' ', confLine);

      if (strncmp (confLine, "purge_after", 9) == 0)
	{

	  tokenPtr = strtok (confLine, "=");
	  tokenPtr = strtok (NULL, "=");
	  // buf_check (tokenPtr, 4096);
	  erase_char (' ', tokenPtr);
	  buf_check (tokenPtr, 6);
	  purge_after = atoi (tokenPtr);
	}
      else
	/* if (strncmp ("PROTECT", confLine, 7) == 0)
	   parse_protected (confLine); */

      if (wasteNum < WASTENUM_MAX)
	{
	  if (strncmp ("WASTE", confLine, 5) == 0)
	    {

	      tokenPtr = strtok (confLine, "=");
	      tokenPtr = strtok (NULL, "=");

	      trim_slash (tokenPtr);
	      erase_char (' ', tokenPtr);

	      change_HOME (tokenPtr);

	      buf_check_with_strop (W_cfg[wasteNum], tokenPtr, CPY);

	      // Make WASTE/files string
	      /* No need to check boundaries for the copy */
	      strcpy (W_files[wasteNum], W_cfg[wasteNum]);
	      buf_check_with_strop (W_files[wasteNum], "/files/", CAT);

	      // Make WASTE/info string
	      /* No need to check boundaries for the copy */
	      strcpy (W_info[wasteNum], W_cfg[wasteNum]);
	      buf_check_with_strop (W_info[wasteNum], "/info/", CAT);

	      // Create WASTE if it doesn't exit
	      waste_check (W_cfg[wasteNum]);

	      // Create WASTE/files if it doesn't exit
	      waste_check (W_files[wasteNum]);

	      // Create WASTE/info if it doesn't exit
	      waste_check (W_info[wasteNum]);

	      // get device number to use later for rename
	      lstat (W_cfg[wasteNum], &st);
	      W_devnum[wasteNum] = st.st_dev;
	      if (list)
		printf ("%s\n", W_cfg[wasteNum]);
	      wasteNum++;

	    }
	}
      else if (wasteNum == WASTENUM_MAX)
	printf ("Maximum number Waste folders reached: %d\n", WASTENUM_MAX);
    }
  fclose (cfgPtr);

  // No WASTE= line found in config file
  if (wasteNum == 0)
    {
      fprintf (stderr, "Configuration file (%s) error; exiting...\n",
	       configFile);
      exit (1);
    }

  return purge_after;

  // From this point on, wasteNum must not be altered
  /* UPDATE 2016-08-03.. Create a non-global from wasteNum */

}

bool
pre_rmw_check (const char *cmdargv, char *file_basename, char *cur_file)
{

  bool onDrive = 0;

  buf_check (cmdargv, MP);

  onDrive = file_exist (cmdargv);
  if (onDrive == 1)
    {
      fprintf (stderr, "File not found: '%s'\n", cmdargv);
      return 1;
    }
  else
    {

      strcpy (cur_file, cmdargv);
      strcpy (file_basename, basename (cur_file));
      return isProtected (cur_file);
    }

}

int remove_to_waste (char *file_basename, char *cur_file)
{

  struct stat st;
  char finalDest[MP];
  int i = 0;
  bool match = 0;
  short statRename = 0;
  bool info_st = 0;
  bool dfn = 0;

  // cycle through wasteDirs to see which one matches
  // device number of file

  for (i = 0; i < wasteNum; i++)
    {
      lstat (cur_file, &st);
      if (W_devnum[i] == st.st_dev)
	{
	  // used by mkinfo
	  curWasteNum = i;
	  buf_check_with_strop (finalDest, W_files[i], CPY);
	  buf_check_with_strop (finalDest, file_basename, CAT);
	  // If a duplicate file exists
	  if (file_exist (finalDest) == 0)
	    {
	      // append a time string
	      buf_check_with_strop (finalDest, time_str_appended, CAT);

	      // tell make info there's a duplicate
	      dfn = 1;
	    }

	  statRename = rename (cur_file, finalDest);
	  if (statRename == 0)
	    {
	      printf ("'%s' -> '%s'\n", cur_file, finalDest);
	      fprintf (undo_file_ptr, "%s\n", finalDest);

	      info_st = mkinfo (dfn, file_basename, cur_file);
	    }
	  else
	    {
	      fprintf (stderr, "Move error or permission denied for '%s'\n",
		       cur_file);
	    }

	  match = 1;
	  // match found (same filesystem). Break from for loop
	  break;
	}

    }				// end -for

  if (!match && !statRename)
    {
      statRename = 1;
      printf ("No suitable filesystem found for \"%s\"\n", cur_file);
    }

  /*    if ( rename(cur_file,cmd) != 0)
     {
     printf("\nFile on different filesystem\n\n");
     int cpErr;
     printf("Executing cp(cur_file,cmd)\n");
     cpErr = cp(cur_file,cmd);
     if (cpErr == 0)
     {
     int unlinkErr;
     printf("Executing unlink(cur_file)\n");
     unlinkErr = unlink(cur_file);
     printf("Err code for unlink is %d\n",unlinkErr);
     }
     else
     printf("cp presented an error of %d\n\n",cpErr);
     }
   */

  return statRename + info_st;

}

int mkinfo (bool dup_filename, char *file_basename, char *cur_file)
{
	FILE *fp;
	 bool bufstat = 0;
	 char finalInfoDest[PATH_MAX + 1];

	 bufstat = buf_check_with_strop (finalInfoDest, W_info[curWasteNum], CPY);
	 bufstat = buf_check_with_strop (finalInfoDest, file_basename, CAT);

	if (dup_filename)
	{
		buf_check_with_strop (finalInfoDest, time_str_appended, CAT);
	}

	bufstat = buf_check_with_strop (finalInfoDest, DOT_TRASHINFO, CAT);

	char real_path[PATH_MAX + 1];
	realpath (cur_file, real_path);
	fp = fopen (finalInfoDest, "w");

	if (fp != NULL)
	{
		fprintf (fp, "[Trash Info]\n");
		fprintf (fp, "Path=%s\n", real_path);
		fprintf (fp, "DeletionDate=%s", time_now);
		fclose (fp);
		return 0;
	}
	else
    {
		printf ("Unable to create info file: %s\n", finalInfoDest);
		printf ("Press the enter key to continue...");
		getchar ();

		return 1;
	}
}

void
Restore (int argc, char *argv[], int optind)
{

  FILE *fp;
  int i;
  char fn_to_restore[MP];
  bool f_state = 0;

  struct filespec
  {
    char *bfn;
    char rp[MP];
    char ip[MP];
  } r;

  char *tokenPtr;
  // adding 5 for the 'Path=' preceding the path.
  char line[MP + 5];

  for (i = optind; i < argc; i++)
    {

      if (file_exist (argv[i]) == 0)
	{
	  buf_check (argv[i], PATH_MAX);
	  realpath (argv[i], r.rp);
	  r.bfn = basename (argv[i]);

	  truncate_str (r.rp, strlen ("files/") + strlen (r.bfn));
	  strcpy (r.ip, r.rp);
	  strcat (r.ip, "info/");
	  strcat (r.ip, r.bfn);
	  strcat (r.ip, DOT_TRASHINFO);

	  f_state = file_exist (r.ip);
	  if (f_state != 0)
	    {
	      printf ("no info file found for %s\n", argv[i]);
	      break;
	    }
	  else
	    {
	      fp = fopen (r.ip, "r");
	      if (fp == NULL)
		{
		  fprintf (stderr, "Error opening info file: %s\n", r.ip);
		  break;
		}
	      else
		{
		  // Not using the "[Trash Info]" line, but reading the file
		  // sequentially
		  if (fgets (line, 14, fp) == NULL)
		    {
		      fprintf (stderr, "Error reading restore file %s\n",
			       r.ip);
		      fclose (fp);
		      break;
		    }
		  else if (strncmp (line, "[Trash Info]", 12) != 0)
		    {
		      fprintf (stderr,
			       "Info file error; format not correct (Line 1): %s\n",
			       r.ip);
		      fclose (fp);
		      break;
		    }

		  // adding 5 for the 'Path=' preceding the path.
		  if (fgets (line, MP + 6, fp) != NULL)
		    {
		      tokenPtr = strtok (line, "=");
		      tokenPtr = strtok (NULL, "=");
		      // tokenPtr now equals the absolute path from the info file
		      // truncating '\n'
		      buf_check_with_strop (fn_to_restore, tokenPtr, CPY);
		      tokenPtr = NULL;
		      trim (fn_to_restore);
		      fclose (fp);

		    }
		  else
		    {
		      printf ("error on line 2 in %s\n", r.ip);
		      fclose (fp);
		      break;
		    }

		  /* Check for duplicate filename */
		  bool onDrive = file_exist (fn_to_restore);

		  if (onDrive == 0)
		    {
		      buf_check_with_strop (fn_to_restore, time_str_appended,
					    CAT);
		      if (verbose == 1)
			{
			  fprintf (stdout,
				   "Duplicate filename at destination - appending time string...\n");
			}

		    }

		  /* end check                                  */

		  if (rename (argv[i], fn_to_restore) == 0)
		    {
		      printf ("+'%s' -> '%s'\n", argv[i], fn_to_restore);
		      if (remove (r.ip) != 0)
			{
			  fprintf (stderr, "error removing info file: '%s'\n",
				   r.ip);
			}
		      else
			{
			  if (verbose)
			    printf ("-%s\n", r.ip);
			}
		    }

		  else
		    {
		      fprintf (stderr, "Restore failed: %s\n", fn_to_restore);
		    }
		}
	    }
	}
      else
	{
	  printf ("%s not found\n", argv[i]);
	}
    }
}

bool
purgeD (void)
{

  FILE *fp;

  char purgeDpath[MP];
  /* buf_check_with_strop(purgeDpath, getenv("HOME"), CPY); */
  buf_check_with_strop (purgeDpath, HOMEDIR, CPY);
  buf_check_with_strop (purgeDpath, PURGE_DAY_FILE, CAT);
  char lastDay[3];
  char nowD[3];
  // a check for buffer overflow is in the get_time_string() function
  get_time_string (nowD, 3, "%d");

  // Already been checked for a buffer overflow, just want to add a NULL
  // terminator, in case something got hosed.
  trim (purgeDpath);

  if (file_exist (purgeDpath) == 0)
    {
      fp = fopen (purgeDpath, "r");
      fgets (lastDay, 3, fp);
      buf_check (lastDay, 3);
      trim (lastDay);
      fclose (fp);

      if (!strcmp (nowD, lastDay))
	// Same day
	return 0;

      // Days differ, run purge
      else
	{
	  fp = fopen (purgeDpath, "w");
	  if (fp != NULL)
	    {
	      fprintf (fp, "%s\n", nowD);
	      fclose (fp);
	      return 1;
	    }
	  else
	    {
	      fprintf (stderr, "Unknown error creating %s\n", purgeDpath);
	      exit (1);
	    }
	}

    }
  else
    {
      // Create file if it doesn't exist and write DD to it.
      fp = fopen (purgeDpath, "w");

      if (fp != NULL)
	{
	  fprintf (fp, "%s\n", nowD);
	  return 1;
	  fclose (fp);
	}
      else
	{
	  fprintf (stderr, "Unknown error creating %s\n", purgeDpath);
	  exit (1);
	}
    }
}

int purge (int purge_after)
{

  if (purge_after > UINT_MAX)
    {
      printf ("purge_after can't be greater than %u\n", UINT_MAX);
      exit (1);
    }

  struct stat st;
  // int i = 0;
  int p = 0;
  bool success = 0;
  struct dirent *entry;

  char *tokenPtr;

  int numPurged = 0;

  char purgeFile[MP];

  struct tm tmPtr, tm_then;
  time_t now;
  time_t then;

  strptime (time_now, "%Y-%m-%dT%H:%M:%S", &tmPtr);
  now = mktime (&tmPtr);

  printf ("\nPurging files older than %u days...\n", purge_after);

  /* Read each Waste directory */
  while (p < wasteNum && p < WASTENUM_MAX)
    {

      DIR *dir = opendir (W_info[p]);
      /* Read each file/dir in Waste directory */
      while ((entry = readdir (dir)) != NULL)
	{

	  buf_check (entry->d_name, MP);

	  if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..")
	      != 0)
	    {
	      FILE *info_file_ptr;
	      const short timeLine = 40;
	      char entry_path[MP];
	      char infoLine[MP + 5];
	      // char *infoLine = NULL;
	      trim (entry->d_name);
	      buf_check_with_strop (entry_path, W_info[p], CPY);
	      buf_check_with_strop (entry_path, entry->d_name, CAT);

	      info_file_ptr = fopen (entry_path, "r");
	      if (info_file_ptr != NULL)
		{
		  // unused  and unneeded Trash Info line
		  fgets (infoLine, 14, info_file_ptr);
		  if (strncmp (infoLine, "[Trash Info]", 12) != 0)
		    {
		      fprintf (stderr,
			       "Info file error; format not correct (Line 1)\n");
		      exit (1);
		    }

		  // The second line is unneeded at this point
		  fgets (infoLine, MP + 5, info_file_ptr);

		  if (strncmp (infoLine, "Path=", 5) != 0)
		    {
		      fprintf (stderr,
			       "Info file error; format not correct (Line 2) : %s\n",
			       entry_path);
		      exit (1);
		    }

		  // The third line is needed for the deletion time
		  fgets (infoLine, timeLine, info_file_ptr);
		  buf_check (infoLine, 40);
		  trim (infoLine);
		  if (strncmp (infoLine, "DeletionDate=", 13) != 0
		      || strlen (infoLine) != 32)
		    {
		      fprintf (stderr,
			       "Info file error; format not correct (Line 3)\n");
		      exit (1);
		    }

		  fclose (info_file_ptr);

		}
	      else
		{
		  printf ("^ An unknown error occurred");
		  exit (1);
		}

	      tokenPtr = strtok (infoLine, "=");
	      tokenPtr = strtok (NULL, "=");

	      strptime (tokenPtr, "%Y-%m-%dT%H:%M:%S", &tm_then);
	      then = mktime (&tm_then);

	      if (then + (86400 * purge_after) <= now)
		{
		  // if (then  <= now) { /* For debugging */
		  success = 0;
		  strcpy (purgeFile, W_files[p]);
		  char temp[MP];
		  strcpy (temp, entry->d_name);
		  truncate_str (temp, strlen (DOT_TRASHINFO));
		  // printf("%s\n", temp);
		  strcat (purgeFile, temp);

		  lstat (purgeFile, &st);

		  if (S_ISDIR (st.st_mode))
		    {
		      rmdir_recursive (purgeFile, 1);
		      if (rmdir (purgeFile) != 0)
			{
			  printf ("Unknown error purging '%s'\n", purgeFile);
			}
		      else
			{
			  success = 1;
			}

		    }
		  else
		    {

		      if (remove (purgeFile) != 0)
			{
			  printf ("Unknown error purging '%s'\n", purgeFile);
			}
		      else
			{
			  success = 1;
			}
		    }

		  if (success)
		    {
		      printf ("-%s\n", purgeFile);
		      if (remove (entry_path) != 0)
			{
			  printf ("Unknown error deleting '%s'\n",
				  entry_path);
			}
		      if (verbose)
			{
			  printf ("-%s\n", entry_path);
			}
		      numPurged++;

		    }
		}
	    }
	}

      closedir (dir);

      p++;
    }

  printf ("%d files purged\n", numPurged);
  return 0;

}


void
undo_last_rmw (void)
{
  FILE *undo_file_ptr;
  char undo_path[MP];
  char line[MP];
  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char */
  char *destiny[1];
  buf_check_with_strop (undo_path, getenv ("HOME"), CPY);
  buf_check_with_strop (undo_path, UNDOFILE, CAT);
  undo_file_ptr = fopen (undo_path, "r");
  if (undo_file_ptr == NULL)
    {
      fprintf (stderr, "Error opening %s for reading\n", undo_path);
      exit (1);
    }
  while (fgets (line, MP - 1, undo_file_ptr) != NULL)
    {
      trim (line);

      destiny[0] = line;

      /* using 0 for third arg so 'for' loop in Restore() will run
       * at least once */
      Restore (1, destiny, 0);
    }
  fclose (undo_file_ptr);

}




/* getch() and getche()
 AUTHOR: zobayer
 */
/* reads from keypress, doesn't echo */
int
getch (void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr (STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar ();
  tcsetattr (STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}

/* reads from keypress, echoes */
int
getche (void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr (STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON);
  tcsetattr (STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar ();
  tcsetattr (STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}



/*
 int cp(const char *from, const char *to) { */
/* Sourced from user caf */
/* http://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c */
/*
 int fd_to, fd_from;
 char buf[4096];
 ssize_t nread;
 int saved_errno;

 fd_from = open(from, O_RDONLY);
 if (fd_from < 0)
 return -1;

 fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
 if (fd_to < 0)
 goto out_error;

 while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
 char *out_ptr = buf;
 ssize_t nwritten;

 do {
 nwritten = write(fd_to, out_ptr, nread);

 if (nwritten >= 0) {
 nread -= nwritten;
 out_ptr += nwritten;
 } else if (errno != EINTR) {
 goto out_error;
 }
 } while (nread > 0);
 }

 if (nread == 0) {
 if (close(fd_to) < 0) {
 fd_to = -1;
 goto out_error;
 }
 close(fd_from);
 // Success
 return 0;
 }

 out_error: saved_errno = errno;

 close(fd_from);
 if (fd_to >= 0)
 close(fd_to);

 errno = saved_errno;
 return -1;
 }
 */

int
rmdir_recursive (char *path, bool isTop)
{

  // static unsigned char depth = 0;

  struct stat st;
  struct dirent *entry;
  DIR *dir;
  char dir_path[PATH_MAX + 1];

  dir = opendir (path);
  while ((entry = readdir (dir)) != NULL)
    {

      /*        if (isTop)
         depth = 0;  */

      /*        if (depth++ >= DEPTH_MAX) {
         printf("Cannot descend into directory more than %d levels.\n",
         DEPTH_MAX);
         printf("Can't remove %s\n", path);
         break;
         }
       */

      strcpy (dir_path, path);

      short pathLen = strlen (dir_path);
      if (dir_path[pathLen - 1] != '/')
	{
	  dir_path[pathLen] = '/';
	  pathLen++;
	  dir_path[pathLen] = '\0';
	}

      strcat (dir_path, entry->d_name);
      if (strcmp (entry->d_name, ".") != 0
	  && strcmp (entry->d_name, "..") != 0)
	{

	  lstat (dir_path, &st);
	  if (S_ISDIR (st.st_mode))
	    {
	      rmdir_recursive (dir_path, 0);
	      // depth--;
	    }
	  else
	    remove (dir_path);
	}
    }
  closedir (dir);
  if (!isTop)
    {
      rmdir (path);
    }

  // need to do some error checking in this function
  return 0;

}

void
mkdirErr (const char *dirname, const char *text_string)
{
  fprintf (stderr, "Unable to create directory: '%s'\n", dirname);
  fprintf (stderr, "%s", text_string);
  exit (1);
}

void
waste_check (const char *p)
{
  struct stat st;
  if (stat (p, &st) != 0)
    {
      if (mkdir (p, S_IRWXU))
	{
	  mkdirErr (p, "Check your configuration file or permissions\n");
	}
    }
}

bool isProtected (char *cur_file)
{

  if (bypass == 0)
    {
      int i;
      char rp[MP];
      realpath (cur_file, rp);

      for (i = 0; i < wasteNum; i++)
	{
	  short len = strlen (W_cfg[i]);
	  if (strncmp (rp, W_cfg[i], len) == 0)
	    break;

	}

      if (i == wasteNum)
	{
	  return 0;
	}
      else
	{
	  printf ("File is in protected directory: %s\n", W_cfg[i]);
	  return 1;
	}
    }
  else
    /* If bypass == 0 */
    return 0;

}

/* void parse_protected (char *line) {
 int i = 0;
 char *tokenPtr;
 tokenPtr = strtok (line, "=");
 tokenPtr = strtok (NULL, "=");
 tokenPtr = strtok (tokenPtr, ",");

 printf ("tokenPtr = '%s'\n", tokenPtr);
 trim (tokenPtr);
 //strncpy (protected[0], tokenPtr, strlen(tokenPtr) );
 strcpy (protected[0], tokenPtr);
 //protected[0] = tokenPtr;
 strcpy (protected[1], tokenPtr);
 // printf("zero = %s\n", protected[0]);
 // tokenPtr = strtok (NULL, ",");
 // protected[1] = tokenPtr;
 //for ( i = 0; i < 2; i++)
 printf("tokenPtr = %s\n", protected[0]);
 printf("tokenPtr = %s\n", protected[1]);
 } */

void
restore_select (void)
{
  struct stat st;
  struct dirent *entry;
  char path_to_file[MP];
  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char */
  char *destiny[1];
  unsigned count = 0;
  int w = 0;
  char input[10];
  char c;
  unsigned char char_count = 0;
  short choice = 0;

  while (w < wasteNum)
    {
      DIR *dir = opendir (W_files[w]);
      count = 0;
      if (!choice)
	{
	  printf ("\t>-- %s --<\n", W_files[w]);

	}
      while ((entry = readdir (dir)) != NULL)
	{
	  if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..")
	      != 0)
	    {
	      count++;
	      if (count == choice || choice == 0)
		{
		  buf_check_with_strop (path_to_file, W_files[w], CPY);
		  /* Not yet sure if 'trim' is needed yet; using it
		   *  until I get smarter */
		  trim (entry->d_name);
		  buf_check_with_strop (path_to_file, entry->d_name, CAT);
		  trim (path_to_file);
		  lstat (path_to_file, &st);
		}
	      if (count == choice)
		{
		  destiny[0] = path_to_file;
		  printf ("\n");
		  /* using 0 for third arg so 'for' loop in Restore() will run
		   *  at least once */
		  Restore (1, destiny, 0);
		  break;
		}
	      if (!choice)
		{
		  printf ("%3d. %s", count, entry->d_name);
		  if (S_ISDIR (st.st_mode))
		    printf (" (D)");
		  if (S_ISLNK (st.st_mode))
		    printf (" (L)");
		  printf ("\n");
		}

	    }
	}
      closedir (dir);
      if (choice)
	break;

      do
	{

	  printf
	    ("Input number to restore, 'enter' to continue, 'q' to quit) ");
	  char_count = 0;
	  input[0] = '\0';
	  choice = 0;

	  while ((c = getche ()) != '\n' && char_count < 9 && c >= '0' && c
		 <= '9')
	    {
	      input[char_count++] = c;
	    }

	  if (c == 'q' && char_count == 0)
	    break;

	  if (c != '\n')
	    {
	      char_count = 0;
	    }

	  if (c == '\n' && char_count == 0)
	    {
	      break;
	    }

	  if (char_count == 0)
	    {
	      printf ("\n");
	    }
	  else
	    {

	      input[char_count] = '\0';
	      choice = atoi (input);
	    }

	}
      while (choice > count || choice < 1);

      /* If user selects 'q' to abort */
      if (c == 'q')
	{
	  printf ("\n");
	  break;
	}

      if (choice == 0)
	w++;

    }

}

bool
file_exist (const char *filename)
{
  struct stat st;
  bool r = 0;
  r = lstat (filename, &st);
  if (r == 0 || S_ISLNK (st.st_mode))
    return 0;
  else
    return 1;

  /* FILE *fp;
     fp = fopen(filename, "r");
     if (fp != NULL) {
     fclose(fp);
     return 0;
     } else {

     return 1;
     }  */
}

void
get_time_string (char *tm_str, unsigned short len, const char *format)
{

  struct tm *TimePtr;
  time_t now = time (NULL);
  TimePtr = localtime (&now);
  strftime (tm_str, len, format, TimePtr);
  buf_check (tm_str, len);
  trim (tm_str);

}
