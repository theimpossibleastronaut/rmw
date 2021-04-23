---
title: Extra Tips
layout: default
---

**To increase verbosity from make**

make V=1

**To view the days remaining until a file is purged**

Use -vv (with -fg)

**To view disk usage used by the files in the Waste folders**
<p class="w3-code">
du -sh `rmw -l`
</p>
(Be sure to use backticks)
