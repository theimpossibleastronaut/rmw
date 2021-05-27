---
title: FAQ
layout: default
---

Anything that isn't covered here may be found on the <a
href="/extra_tips.html">Extra Tips</a> page.

**What is your release cycle?**

It varies, averaging once or twice a year. If bugs are found and
reported, a release will happen sooner depending on the severity.
Upcoming releases can be tracked using the [Milestones
page](https://github.com/theimpossibleastronaut/rmw/milestones). The
dates may change but Andy tries to keep them as up-to-date as
possible.

**Can rmw replace rm? Can I alias rmw to rm?**

I don't recommend it. Many other utilities use rm in the background and
you'd wind up with a very full trash can. Also, rmw doesn't have the
same command line option as rm (see also: an <a
href="https://github.com/theimpossibleastronaut/rmw/discussions/305">extended
discussion</a>)

**How do I know if rmw is compatible with my Desktop trash?**

When rmw moves a file to a waste or trash directory, it also writes a
*.trashinfo* file to the corresponding trash directory. The default
waste directory that rmw uses is *~/.local/share/Waste*, therefore
<code class="w3-codespan">rmw foo</code> would result in

<p class="w3-code">
  ~/.local/share/Waste/files/foo<br />
  ~/.local/share/Waste/info/foo.trashinfo
</p>

<a id="dot_trashinfo">The contents of *foo.trashinfo* would look like this:</a>

<p class="w3-code">
  [Trash Info]<br />
  Path=/home/andy/src/rmw-project/rmw/foo<br />
  DeletionDate=2019-07-03T16:48:47
</p>

On most *nix and *BSD systems, the desktop trash [has the same
format](https://specifications.freedesktop.org/trash-spec/trashspec-latest.html),
and is located in **~/.local/share/Trash**. You can verify that the
trashinfo format and directory layout is the same.

If you're sure that your Desktop trash is compatible, you can add the
appropriate line to your rmw configuration file.

**Does rmw work on Windows?**

Not yet. There's [an open
ticket](https://github.com/theimpossibleastronaut/rmw/issues/71) for
that. But reportedly, rmw works well on the <a
href="https://github.com/ethanhs/WSL-Programs">Windows Subsystem for
Linux</a>.

**Can I use wildcard and regex patterns with rmw?**

Yes. For example:

<p class="w3-code">
  rmw *.txt<br />
  rmw test[0-9].txt
</p>

Some complex regex expressions won't work. If you'd like support
for a particular pattern that doesn't already work, please open a
ticket.

**Can rmw be run as a scheduled job to purge expired files?**

I wouldn't recommend it. It's never been tested that way, but more
importantly, it could cause a <a
href="https://devopedia.org/race-condition-software">race
condition</a>. For example, if you manually run rmw at the same time
it's being run by a task scheduler, rmw could have unpredictable
results as it tries to read files that are in the process of being
removed (the rare possibility of collision exists even though rmw
usually only takes a few seconds or less to run).
