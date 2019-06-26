---
title: Translating
layout: default
---

The rmw project uses [GNU
gettext](https://www.gnu.org/software/gettext/manual/html_node/index.html)
for translations. The po files are in
[master/po](https://github.com/theimpossibleastronaut/rmw/tree/master/po).
Any changes you make or translations you add can be submitted via a
normal [pull
request](https://github.com/theimpossibleastronaut/rmw/blob/master/CONTRIBUTING.md#pull-requests).
If you've never done software translation before and have questions,
you can stop by our [chat
room](/index.html#support) or open an
issue. You can edit a po file with any text editor or use one of the utilities listed below. To initialize a new po file:

<code class="w3-codespan">  msginit -l ??</code> (Where ?? is your iso-631-1 language code)

Example: To create a french .po file, run <code class="w3-codespan">msginit -l fr</code>

## Po editing utilities

* [PoEdit](https://poedit.net/)
* [GTranslator](https://wiki.gnome.org/Apps/Gtranslator)
