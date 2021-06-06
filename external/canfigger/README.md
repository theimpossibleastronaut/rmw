# canfigger v0.1.0

Simple configuration file parser library

[![codeql-badge]][codeql-url]
[![actions-c-badge]][actions-c-url]

[codeql-badge]: https://github.com/andy5995/canfigger/workflows/CodeQL/badge.svg
[codeql-url]: https://github.com/andy5995/canfigger/actions?query=workflow%3ACodeQL
[actions-c-badge]: https://github.com/andy5995/canfigger/actions/workflows/c-cpp.yml/badge.svg
[actions-c-url]: https://github.com/andy5995/canfigger/actions/workflows/c-cpp.yml

website: https://github.com/andy5995/canfigger

This library contains a function that parses simple configuration files
that use a key/value pair with an optional attribute.

```
foo=bar
blue=color,shiny

# Spaces adjacent to the '=' or the attribute delimiter ',' will be ignored.
# Leading tabs will be ignored.
		statement = hello world , obvious

# An option with no value or attributes
FeatureFooEnabled
```

`canfigger_parse_file()` returns a [linked
list](https://www.learn-c.org/en/Linked_lists), each node in the list
corresponds to a parsed line in your configuration file.

## Example usage

```c
#include <stdio.h>
#include "canfigger.h"

int
main (void)
{
  // call the primary library function to read your config file
  st_canfigger_list *list = canfigger_parse_file ("test_file.conf", ',');

  // create a pointer to the head of the list before examining the list.
  st_canfigger_list *root = list;
  if (list == NULL)
  {
    fprintf (stderr, "Error");
    return -1;
  }

  int i = 0;
  while (list != NULL)
  {
    printf ("\n\
Key: %s\n\
Value: %s\n\
Attribute: %s\n", list->key, list->value, list->attribute);

    i++;

    list = list->next;
  }

  // free the list
  canfigger_free (root);

  return 0;
}
```

## API

```c
//
// Opens a config file and returns a memory-allocated linked list
// that must be freed later (see canfiggerfree())
//
// Each node is of type st_canfigger_node.
st_canfigger_list *canfigger_parse_file (const char *file,
                                         const char delimiter);


//
// Frees the list returned by canfigger_parse_file()
void canfigger_free (st_canfigger_node * node);


typedef struct st_canfigger_node st_canfigger_node;
struct st_canfigger_node
{
  // Contains the string that precedes the '=' sign
  char *key;

  // Contains the string between the '=' sign and the delimiter
  char *value;

  // Contains the string following the delimiter
  char *attribute;

  // A pointer to the next node in the list
  st_canfigger_node *next;
};


String containing the version of the library
CANFIGGER_VERSION
```

## Building

```
meson _build
cd _build
ninja
```

## Run the tests

```
ninja test
meson test --setup=valgrind (requires valgrind to be installed)
```

## Install/Uninstall

```
ninja install
```

Or if you want to install without superuser privileges, first run

    meson -Dprefix=$HOME/.local _build

```
ninja uninstall
```
