/*
 * primary_funcs.c
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

#include "function_prototypes.h"


/* Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 * */

bool
buf_check_with_strop (char *s1, const char *s2, bool mode)
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

bool
buf_check (const char *str, unsigned short boundary)
{

  unsigned short len = strlen (str);

  if (len >= boundary)
  {
    printf ("Potential buffer overrun caught; rmw terminating.\n");
    exit (1);
  }

  return 0;
}

bool
pre_rmw_check (const char *cmdargv, char *file_basename, char *cur_file,
               struct waste_containers *waste, bool bypass, const int wdt,
               char pro_dir[PROTECT_MAX][MP], const int pro_tot)
{
  buf_check (cmdargv, MP);

  if (!file_not_found (cmdargv))
  {
    strcpy (cur_file, cmdargv);
    strcpy (file_basename, basename (cur_file));
    return isProtected (cur_file, waste, bypass, wdt, pro_dir, pro_tot);
  }

  fprintf (stderr, "File not found: '%s'\n", cmdargv);
  return 1;
}

bool
isProtected (char *cur_file, struct waste_containers *waste, bool bypass,
            const int wdt, char pro_dir[PROTECT_MAX][MP], const int pro_tot)
{
  if (bypass)
    return 0;

  int waste_dir;
  char rp[MP];
  realpath (cur_file, rp);

  bool flagged = 0;

  for (waste_dir = 0; waste_dir < wdt; waste_dir++)
  {
    short len = strlen (waste[waste_dir].parent);
    if (strncmp (rp, waste[waste_dir].parent, len) == 0)
    {
      flagged = 1;
      break;
    }
  }

  short dir_num;

  for (dir_num = 0; dir_num < pro_tot; dir_num++)
  {
    if (!strcmp (rp, pro_dir[dir_num]))
    {
      flagged = 1;
      break;
    }
  }

  if (flagged)
    printf ("Protected directory: %s\n", rp);

  return flagged;


}

int
mkinfo (bool dup_filename, char *file_basename, char *cur_file,
        struct waste_containers *waste, char *time_now,
        char *time_str_appended, const short cnum)
{
  FILE *fp;
  bool bufstat = 0;
  char finalInfoDest[PATH_MAX + 1];

  bufstat =
    buf_check_with_strop (finalInfoDest, waste[cnum].info, CPY);

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
undo_last_rmw (const char *HOMEDIR, char *time_str_appended)
{
  FILE *undo_file_ptr;
  char undo_path[MP];
  char line[MP];
  /* using destiny because the second arg for Restore() must be
   * a *char[] not a *char */
  char *destiny[1];
  buf_check_with_strop (undo_path, HOMEDIR, CPY);
  buf_check_with_strop (undo_path, UNDO_FILE, CAT);
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
    Restore (1, destiny, 0, time_str_appended);
  }

  fclose (undo_file_ptr);
}

/**
 * getch() and getche()
 * AUTHOR: zobayer
 *
 * reads from keypress, doesn't echo
 */
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



bool
file_not_found (const char *filename)
{
  struct stat st;
  bool r = 0;
  r = lstat (filename, &st);
  if (!r || S_ISLNK (st.st_mode))
    return 0;
  else
    return 1;
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

int
remove_to_waste (char *cur_file, char *file_basename,
                 struct waste_containers *waste, char *time_now,
                 char *time_str_appended, FILE * undo_file_ptr, const int wdt)
{
  struct stat st;
  char finalDest[MP];
  int i = 0;
  bool match = 0;
  short statRename = 0;
  bool info_st = 0;
  bool dfn = 0;

  short curWasteNum = 0;

  // cycle through wasteDirs to see which one matches
  // device number of file

  for (i = 0; i < wdt; i++)
  {
    lstat (cur_file, &st);
    if (waste[i].dev_num == st.st_dev)
    {
      // used by mkinfo
      curWasteNum = i;
      buf_check_with_strop (finalDest, waste[i].files, CPY);
      buf_check_with_strop (finalDest, file_basename, CAT);
      // If a duplicate file exists
      if (file_not_found (finalDest) == 0)
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

        info_st = mkinfo (dfn, file_basename, cur_file, waste,
                          time_now, time_str_appended, curWasteNum);
      }
      else
      {
        fprintf (stderr,
                 "Move error or permission denied for '%s'\n", cur_file);
      }

      match = 1;
      // match found (same filesystem). Break from for loop
      break;
    }

  }                             // end -for

  if (!match && !statRename)
  {
    statRename = 1;
    printf ("No suitable filesystem found for \"%s\"\n", cur_file);
  }

  return statRename + info_st;
}
