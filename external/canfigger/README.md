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
foo = bar
blue = green, color
# Commented line

FeatureFoo-enabled
```

`canfigger_parse_file()` returns a [linked
list](https://www.learn-c.org/en/Linked_lists), each node in the list
corresponds to a parsed line in your configuration file.

## Example usage

```c
  st_canfigger_list *list = canfigger_parse_file ("../test_canfigger.conf", ',');

  // create a pointer to the first node in the list before moving through the list
  st_canfigger_list *root = list;

  if (list == NULL)
  {
    fprintf (stderr, "Error");
    return -1;
  }

  while (list != NULL)
  {
    printf ("\n\
Key: %s\n\
Value: %s\n\
Attribute: %s\n", list->key, list->value, list->attribute);

    list = list->next;
  }

  // pass the root pointer to canfigger_free when done
  canfigger_free (root);
```

## API

**CANFIGGER_VERSION** String containing the version of the library

`st_canfigger_list *canfigger_parse_file (const char *file, const char delimiter)`

`void canfigger_free (st_canfigger_node *node)`

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
