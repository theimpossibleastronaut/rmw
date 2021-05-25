---
title: Translating
layout: default
---

The rmw project uses [Transifex](https://www.transifex.com/aalt/rmw/)
or [GNU
gettext](https://www.gnu.org/software/gettext/manual/html_node/index.html)
for translations. If you're not using Transifex, you can check if a po
file already exists for your language in
[master/po](https://github.com/theimpossibleastronaut/rmw/tree/master/po).
Any changes you make or translations you add can be submitted via a
normal [pull
request](https://github.com/theimpossibleastronaut/rmw/blob/master/CONTRIBUTING.md#pull-requests).

If you are using Transifex, you can submit changes or additions the way
you normally would. Please open an issue on the rmw GitHub repo to let
us know you are working on a translation or have completed it.

If you've never done software translations before and need help, don't worry!
You can ask for guidance using the rmw issues section.

You can edit a po file with any text editor or use one of the utilities listed below. To initialize a new po file:

<code class="w3-codespan">  msginit -l ??</code> (Where ?? is your iso-631-1 language code)

Example: To create a french .po file, run <code class="w3-codespan">msginit -l fr</code>

## Po editing utilities

* [PoEdit](https://poedit.net/)
* [GTranslator](https://wiki.gnome.org/Apps/Gtranslator)
