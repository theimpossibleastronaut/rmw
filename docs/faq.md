---
title: FAQ
layout: default
---

**Can rmw replace rm? Can I alias rmw to rm?**

I don't recommend it. Many other utilities use rm in the background and
you'd wind up with a very full trash can. Also, rmw doesn't have the
same command line option as rm.

**Does rmw work on Windows?**

Not yet. There's [an open
ticket](https://github.com/theimpossibleastronaut/rmw/issues/71) for
that.

**Why is there an option to build rmw as a library?**

A friend of mine wanted to experiment with a library for it, and I
thought maybe one day other people might want to as well.

**Can I use wildcard and regex patterns with rmw?**

Yes. For example:

<p class="w3-code">
  rmw *.txt<br />
  rmw test[0-9].txt
</p>

Some complex regex expressions won't work. If you'd like support
for a particular pattern that doesn't already work, please open a
ticket.
