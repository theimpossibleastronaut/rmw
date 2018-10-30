/* bst.h
 *
 */

#include <strings.h>

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
