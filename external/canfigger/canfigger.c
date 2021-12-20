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
          snprintf (tmp_node->attribute, sizeof tmp_node->attribute, "%s", tmp_attribute);
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
