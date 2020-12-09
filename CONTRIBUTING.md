# Contributing to rmw

## Bug Reports and Feature Requests

Anyone may [open an
issue](https://github.com/theimpossibleastronaut/rmw/issues).

Feature requests should go in [GitHub
Discussions](https://github.com/theimpossibleastronaut/rmw/discussions).

## Coding Standards

The goal is to use a style similar to [GNU Coding
Standards](https://www.gnu.org/prep/standards/html_node/Formatting.html#Formatting),
but braces not indented:

```
  if (cli_user_options.list)
  {
    list_waste_folders (st_config_data.st_waste_folder_props_head);
    return 0;
  }
```

## Website

See [Website Design](https://remove-to-waste.info/website-design.html)
for information specific to the rmw website.

## Patches and Pull Requests

To prevent work-overlap, please post on a ticket if you'll be working
on a specific issue (or create a ticket if there's not one open yet.
**Note**: If more than one person submits a patch for the same thing,
your patch may get rejected.

**Note**: If you agreed to work to work on a ticket but later find that
you're unable to work on it, or if you changed your mind, please post
again on the ticket to let everyone know it's up for grabs.

You can use [The GitHub
flow](https://guides.github.com/introduction/flow/), which mostly just
involves creating a separate branch for each patch you're working on.
Using that method helps prevent merge conflicts later. *Note* that you
should never need to work on the master branch or merge your patches
into the master branch (See "syncing" below).

Source code patches should only contain changes related to a single
issue. This helps speed up the review and discussion process. However,
if you're helping fix typos and grammar errors in documentation,
multiple changes in one PR is fine. General rule of thumb for
documentation patches on this project is 5 unrelated changes or fewer
to a PR. But if they are only one-word or letter changes, I can be
flexible and more than 5 will still be gratefully accepted for review.

If you're submitting a documentation-only PR, please add `[skip ci]` to
the commit message; it will prevent automatic code testing on
[Travis](https://travis-ci.org/theimpossibleastronaut/rmw).

Alternatively, if you've contributed code that doesn't have anything to
do with files in the docs/ directory, add `[skip jekyll]` to the commit
message.

If you submit a pull request, please add yourself (along with your
personal link) to
[AUTHORS.md](https://github.com/theimpossibleastronaut.com/rmw/blob/master/AUTHORS.md)

## Syncing

Periodically, you'll need to sync your repo with the upstream.
GitHub has instructions for doing this

* [Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/)
  * For step 3 on that page, use https://github.com/theimpossibleastronaut.com/rmw.git for the URL.
* [Syncing a Fork](https://help.github.com/articles/syncing-a-fork/)
  * On that page, it shows how to merge the **master** branch (steps 4 & 5).
