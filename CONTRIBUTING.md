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
into the master branch (don't forget to periodically [sync your
repo](https://docs.github.com/en/github/collaborating-with-pull-requests/working-with-forks/syncing-a-fork).)

Source code patches should only contain changes related to a single
issue. This helps speed up the review and discussion process. However,
if you're helping fix typos and grammar errors in documentation,
multiple changes in one PR is fine. General rule of thumb for
documentation patches on this project is 5 unrelated changes or fewer
to a PR. But if they are only one-word or letter changes, I can be
flexible and more than 5 will still be gratefully accepted for review.

If you submit a pull request, please add yourself (along with your
personal link) to
[AUTHORS.md](https://github.com/theimpossibleastronaut.com/rmw/blob/master/AUTHORS.md)

## Code of Conduct

In the interest of fostering an open and welcoming environment, we as
contributors and maintainers would like to making participation in our
project and our community a harassment-free experience for everyone.

### Our Standards

Examples of behavior that contributes to creating a positive
environment include:

* Not being disrespectful of differing viewpoints and experiences
(i.e., keep opinions to yourself if you can't discuss in an open and
friendly way).

* Accept or ignore constructive criticism. The choice is yours. But
don't attack someone who has given you a reasonable opinion of your
code or doc changes.

Examples of unacceptable behavior by participants include:

* The use of sexualized language or imagery and unwelcome sexual attention or advances
* Trolling, insulting/derogatory comments and personal, political, or religious attacks
* Public or private harassment
* Other conduct which could reasonably be considered inappropriate in a professional setting

## Our Responsibilities

Project maintainers are responsible for clarifying the standards of
acceptable behavior and are expected to take appropriate and fair
corrective action in response to any instances of unacceptable
behavior.

Project maintainers have the right and responsibility to remove, edit,
or reject comments, commits, code, wiki edits, issues, and other
contributions that are not aligned to this Code of Conduct, or to ban
temporarily or permanently any contributor for other behaviors that
they deem inappropriate, threatening, offensive, or harmful.

## Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may
be reported by contacting the project maintainer, [Andy
Alt](https://github.com/andy5995) at andy400-dev@yahoo.com. He will
review and investigate all complaints, and will respond in a way that
he deems appropriate to the circumstances. He is obligated to maintain
confidentiality with regard to the reporter of an incident. Further
details of specific enforcement policies may be posted separately.

Ultimately, Mr. Alt can not be held responsible for the actions of
other members, but he will do his best to deal with any reported
incidents.

Furthermore, if you feel that the project maintainer has violated the
Code of Conduct and don't feel comfortable contacting him about it,
please [report him to
GitHub](https://help.github.com/en/articles/reporting-abuse-or-spam).

## Attribution

This Code of Conduct is a heavily modified version of the [Contributor
Covenant][homepage], version 1.4, available at
[http://contributor-covenant.org/version/1/4][version]

[homepage]: http://contributor-covenant.org
[version]: http://contributor-covenant.org/version/1/4/
