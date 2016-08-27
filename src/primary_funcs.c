/*
 * primary_funcs.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
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

#include "primary_funcs.h"


/**
 * buf_check_with_strop()
 *
 * FIXME: This function needs optimizing.
 *
 * Before copying or catting strings, make sure str1 won't exceed
 * PATH_MAX + 1
 */
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

     /**
     * This exit() is related to issue #8
     * https://github.com/andy5995/rmw/issues/8
     */
    exit (-1);

  }

}

bool
buf_check (const char *str, unsigned short boundary)
{
  unsigned short len = strlen (str);

  if (len >= boundary)
  {
    printf ("Potential buffer overrun caught; rmw terminating.\n");

  /**
   * This exit() is related to issue #8
   * https://github.com/andy5995/rmw/issues/8
   */
    exit (1);
  }

  return 0;
}

int
mkinfo (struct rmw_target file, struct waste_containers *waste, char *time_now,
        char *time_str_appended, const short cnum)
{
  FILE *fp;
  bool bufstat = 0;
  char finalInfoDest[PATH_MAX + 1];

  bufstat =
    buf_check_with_strop (finalInfoDest, waste[cnum].info, CPY);

  bufstat = buf_check_with_strop (finalInfoDest, file.base_name, CAT);

  if (file.is_duplicate)
    buf_check_with_strop (finalInfoDest, time_str_appended, CAT);

  bufstat = buf_check_with_strop (finalInfoDest, DOT_TRASHINFO, CAT);

  char real_path[MP];

  if (resolve_path (file.main_argv, real_path))
    return 1;

  fp = fopen (finalInfoDest, "w");

  if (fp == NULL)
  {
    fprintf (stderr, "Error: Unable to create info file :\n");
    fprintf (stderr, "While opening %s for writing :\n", finalInfoDest);
    perror ("mkinfo()");
    return 1;
  }
  else
  {
    fprintf (fp, "[Trash Info]\n");
    fprintf (fp, "Path=%s\n", real_path);
    fprintf (fp, "DeletionDate=%s", time_now);

    if (fclose (fp) == EOF)
    {
      fprintf (stderr, "Error while closing %s :\n", finalInfoDest);
      perror ("mkinfo()");
      return 1;
    }

    return 0;
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

  if (undo_file_ptr != NULL)
  {}

  else
  {
    fprintf (stderr, "Error: while opening %s for reading\n", undo_path);
    perror ("undo_last_rmw()");

    /**
     * This exit() is related to issue #8
     * https://github.com/andy5995/rmw/issues/8
     */
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

/**
 * This will get deleted, and be made part of a function
 * that can make directories when parents don't exist
 */
void
mkdirErr (const char *dirname, const char *text_string)
{
  fprintf (stderr, "Unable to create directory: '%s'\n", dirname);
  fprintf (stderr, "%s", text_string);
  exit (1);
}

/**
 * waste_check()
 *
 * FIXME: Will only create the dir if the parent dir exists
 * use routine from check_for_data_dir() (needs to be converted to
 * a general use function
 *
 * Check for the existence of WASTE dirs that are specified in the config
 * file. Creates them as necessary.
 */
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

/**
 * file_not_found()
 * Checks for the existence of *filename
 *
 * returns 1 if not found, 0 if the file doesn't exist
 */
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
