/*!
 * @file bst.h
 */

#include <strings.h>
#include <stdlib.h>
#include "config.h"
#include "utils_rmw.h"

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

/*! Used for @ref restore_select() and contains info about each file
 * in the waste folder
 */
typedef struct st_node st_node;
struct st_node
{
  /*! Holds the filename */
  char data[LEN_MAX_PATH];

  /*! Holds the human readable size of the file */
  char size_str[LEN_MAX_HUMAN_READABLE_SIZE];

  /*! Left node of the binary search tree */
  st_node *left;

  /*! Right node of the binary search tree */
  st_node *right;
};

st_node *insert_node (st_node * root, comparer compare, char *data, char *size_str);

void populate_menu (st_node * node, ITEM ** my_items, bool level_one);

void dispose (st_node *root);
