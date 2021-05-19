---
title: Extra Tips
layout: default
---

**To increase verbosity from make**

make V=1

**To view disk usage used by the files in the Waste directories**

Warning: this could take a long time to run if you have a lot of items in your waste directories.

You can use the <a href="https://www.man7.org/linux/man-pages/man1/du.1.html">du</a> command:

<p class="w3-code">
du -sh `rmw -l`
</p>
(Be sure to use backticks)
