/*
This file is part of canfigger<https://github.com/andy5995/canfigger>

Copyright (C) 2021  Andy Alt (andy400-dev@yahoo.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

// for canfigger_realize_str()
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#include "canfigger.h"


void
canfigger_free (st_canfigger_node * node)
{
  if (node != NULL)
  {
    canfigger_free (node->next);
    free (node);
  }
  return;
}


/*
 * returns a pointer to the first character after lc
 * If lc appears more than once, the pointer
 * will move past that as well.
 *
 * Ex1: "__Hello World": the pointer will be set to the 'H'.
 * Ex2: "_H_ello World": Again, the pointer will be set to the 'H'.
 */
static char *
erase_lead_char (const char lc, char *haystack)
{
  char *ptr = haystack;
  if (*ptr != lc)
    return ptr;

  while (*ptr == lc)
    ptr++;

  return ptr;
}


/*!
 * Removes trailing white space from a string (including newlines, formfeeds,
 * tabs, etc
 * @param[out] str The string to be altered
 * @return void
 */
static void
trim_whitespace (char *str)
{
  if (str == NULL)
  {
#ifndef TEST_LIB
    exit (EXIT_FAILURE);
#else
    errno = 1;
    return;
#endif
  }

  char *pos_0 = str;
  /* Advance pointer until NULL terminator is found */
  while (*str != '\0')
    str++;

  /* set pointer to segment preceding NULL terminator */
  if (str != pos_0)
    str--;
  else
    return;

  while (isspace (*str))
  {
    *str = '\0';
    if (str != pos_0)
      str--;
    else
      break;
  }

  return;
}


static void
grab_str_segment (const char *begin, const char *end, char *str)
{
  do
  {
    *str++ = *begin++;
  }
  while (begin < end);

  *str = '\0';
  return;
}


st_canfigger_list *
canfigger_parse_file (const char *file, const char delimiter)
{
  static st_canfigger_node *root = NULL;
  st_canfigger_list *list = NULL;

  FILE *fp = fopen (file, "r");
  if (fp == NULL)
    return NULL;

  char line[__CFG_LEN_MAX_LINE];
  while (fgets (line, sizeof line, fp) != NULL)
  {
    trim_whitespace (line);
    char *line_ptr = line;
    line_ptr = erase_lead_char (' ', line_ptr);
    line_ptr = erase_lead_char ('\t', line_ptr);
    switch (*line_ptr)
    {
    case '#':
      continue;
    case '\0':
      continue;
    }

    st_canfigger_node *tmp_node = malloc (sizeof (struct st_canfigger_node));
    if (tmp_node != NULL)
    {
      if (list != NULL)
        list->next = tmp_node;

      *tmp_node->key = '\0';
      *tmp_node->value = '\0';
      *tmp_node->attribute = '\0';

      char *tmp_key = strchr (line_ptr, '=');
      if (tmp_key == NULL)
      {
        trim_whitespace (line_ptr);
        snprintf (tmp_node->key, sizeof tmp_node->key, "%s", line_ptr);
      }
      else
      {
        char key[__CFG_LEN_MAX_LINE];
        grab_str_segment (line_ptr, tmp_key, key);
        trim_whitespace (key);
        snprintf (tmp_node->key, sizeof tmp_node->key, "%s", key);

        tmp_key++;
        tmp_key = erase_lead_char (' ', tmp_key);
        char *r_value = tmp_key;
        char *tmp_value = strchr (r_value, delimiter);
        if (tmp_value == NULL)
        {
          trim_whitespace (r_value);
          snprintf (tmp_node->value, sizeof tmp_node->value, "%s", r_value);
        }
        else
        {
          char value[__CFG_LEN_MAX_LINE];
          grab_str_segment (r_value, tmp_value, value);
          trim_whitespace (value);
          snprintf (tmp_node->value, sizeof tmp_node->value, "%s", value);

          tmp_value++;
          char *tmp_attribute = erase_lead_char (' ', tmp_value);
          snprintf (tmp_node->attribute, sizeof tmp_node->attribute,
                    "%s", tmp_attribute);
        }
      }
      tmp_node->next = NULL;
      list = tmp_node;

      if (root == NULL)
        root = list;
    }
    else
    {
      if (root != NULL)
        canfigger_free (root);

      fclose (fp);
      return NULL;
    }
  }

  int r = fclose (fp);
  if (r != 0)
  {
    return NULL;
  }

  list = root;
  return list;
}


const st_canfigger_directory *
canfigger_get_directories (void)
{
  static st_canfigger_directory st_directory;
  st_directory.home = getenv ("HOME");
  if (st_directory.home == NULL)
    return NULL;

  const char *xdg_configroot = getenv ("XDG_CONFIG_HOME");
  if (xdg_configroot == NULL)
    snprintf (st_directory.configroot,
              sizeof st_directory.configroot,
              "%s/.config", st_directory.home);
  else
    snprintf (st_directory.configroot,
              sizeof st_directory.configroot, "%s", xdg_configroot);

  const char *xdg_dataroot = getenv ("XDG_DATA_HOME");
  if (xdg_dataroot == NULL)
    snprintf (st_directory.dataroot,
              sizeof st_directory.dataroot,
              "%s/.local/share", st_directory.home);
  else
    snprintf (st_directory.dataroot,
              sizeof st_directory.dataroot, "%s", xdg_dataroot);

  return &st_directory;
}

/*!
 * Trim a trailing character of a string
 * @param[in] c The character to erase
 * @param[out] str The string to alter
 * @return void
 */
static void
trim_char (const char c, char *str)
{
  trim_whitespace (str);
  while (*str != '\0')
    str++;

  str--;

  while (*str == c)
  {
    *str = '\0';
    str--;
  }

  return;
}


/*
 * replace part of a string, adapted from code by Gazl
 * https://www.linuxquestions.org/questions/showthread.php?&p=5794938#post5794938
*/
static char *
strrepl (char *src, const char *str, char *repl)
{
  // The replacement text may make the returned string shorter or
  // longer than src, so just add the length of all three for the
  // mallocation.
  size_t req_len = strlen (src) + strlen (str) + strlen (repl) + 1;
  char *dest = malloc (req_len);
  if (dest == NULL)
    return NULL;

  char *s, *d, *p;

  s = strstr (src, str);
  if (s && *str != '\0')
  {
    d = dest;
    for (p = src; p < s; p++, d++)
      *d = *p;
    for (p = repl; *p != '\0'; p++, d++)
      *d = *p;
    for (p = s + strlen (str); *p != '\0'; p++, d++)
      *d = *p;
    *d = '\0';
  }
  else
    strcpy (dest, src);

  dest = realloc (dest, strlen (dest) + 1);
  if (dest == NULL)
    return NULL;

  return dest;
}


unsigned short
canfigger_realize_str (char *str, const char *homedir)
{
  trim_char ('/', str);
  /* Any trailing whitespace should already be trimmed before this function is called */

  uid_t uid = geteuid ();
  struct passwd *pwd = getpwuid (uid);  /* don't free, see getpwnam() for details */

  if (pwd == NULL)
    return -1;


  /* What's a good length for this? */
  char UID[40];
  if ((size_t) snprintf (UID, sizeof UID, "%u", pwd->pw_uid) >= sizeof UID)
    return -1;

  struct st_vars_to_check
  {
    const char *name;
    const char *value;
  } st_var[] = {
    {"~", homedir},
    {"$HOME", homedir},
    {"$UID", UID},
    {NULL, NULL}
  };

  int i = 0;
  while (st_var[i].name != NULL)
  {
    if (strstr (str, st_var[i].name) != NULL)
    {
      char *dest = strrepl (str, st_var[i].name, (char *) st_var[i].value);
      if (dest == NULL)
        return -1;

      if (snprintf (str, PATH_MAX, "%s", dest) >= PATH_MAX)
        return -1;

      free (dest);

      /* check the string again, in case str contains something like
       * $HOME/Trash-$UID (which would be rare, if ever, but... */
      i--;
    }

    i++;
  }

  return 0;
}
