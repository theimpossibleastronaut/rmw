/*
 * str_funcs.c
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

#include "rmw.h"
#include "function_prototypes.h"

// str_funcs.c
// string handling functions for rmw

/* trim: remove trailing blanks, tabs, newlines */
/* Adapted from The ANSI C Programming Language, 2nd Edition (p. 65) */
/* Brian W. Kernighan & Dennis M. Ritchie */
int
trim (char s[])
{
  int n;

  for (n = strlen (s) - 1; n >= 0; n--)
    {
      if (s[n] != ' ' && s[n] != '\t' && s[n] != '\n')
	{
	  // Add null terminator regardless
	  s[n + 1] = '\0';
	  break;
	}
      s[n] = '\0';
    }
  return n;
}

void
erase_char (char c, char *str)
{
  int inc = 0;
  /* int i;
     int n; */

  while (str[inc] == c)
    inc++;

  strcpy (str, str + inc);

  /* n = strlen (str);
     for (i = 0; i < n - inc; i++)
     {
     str[i] = str[i + inc];
     }

     str[n - inc] = '\0'; */

}

/* change_HOME()
 * if "$HOME" or "~" is used on configuration file
 * change to "HOMEDIR" */
bool
change_HOME (char *t)
{
  bool status = 0;
  if (t[0] == '~')
    {
      erase_char ('~', t);

      status = 1;
    }
  else if (strncmp (t, "$HOME", 5) == 0)
    {
      int i;
      for (i = 0; i < 5; i++)
	{
	  erase_char (t[0], t);

	}
      status = 1;
    }

  if (status == 1)
    {
      char temp[MP];
      buf_check_with_strop (temp, HOMEDIR, CPY);
      buf_check_with_strop (temp, t, CAT);
      strcpy (t, temp);

    }

  return status;

}

int
trim_slash (char s[])
{
  int n;
  for (n = strlen (s) - 1; n >= 0; n--)
    {
      if (s[n] != '/')
	{
	  break;
	}
      s[n] = '\0';
    }
  return n;
}


/* truncate_str() */
/* adding the null terminator to chop off part of a string
 * at a given point */
void
truncate_str (char *str, short len)
{
  short offset = strlen (str) - len;
  str[offset] = '\0';
}
