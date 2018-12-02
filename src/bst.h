/* bst.h
 *
 */

#include <strings.h>
#include <stdlib.h>
#include "config.h"

#if defined HAVE_NCURSESW_MENU_H
#  include <ncursesw/menu.h>
#elif defined HAVE_NCURSES_MENU_H
#  include <ncurses/menu.h>
#elif defined HAVE_MENU_H
#  include <menu.h>
#else
#  error "SysV-compatible Curses Menu header file required"
#endif

typedef int (*comparer) (const char *, const char *);

typedef struct node
{
  char *data;
  char *desc;
  struct node *left;
  struct node *right;
} node;

node *insert_node (node * root, comparer compare, char *data, char *desc);

void populate_menu (node * node, ITEM ** my_items, bool level_one);

void dispose (node *root);
