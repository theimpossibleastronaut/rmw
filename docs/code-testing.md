---
title: Code Testing
layout: default
---
<ul>
  <li><a href="#general_testing">General Testing</a></li>
  <li><a href="#env_vars">Environmental Variables</a></li>
</ul>

<h2 id="general_testing">General Testing</h2>

If you're writing a patch, using <code class="w3-codespan">make
check</code> will cover most of rmw's operations. If there's a test
missing, please open a ticket.

To test using [valgrind](https://www.valgrind.org/) (from the build dir):

<code class="w3-codespan">meson test --setup=valgrind</code>

<h2 id="env_vars">Environmental Variables</h2>
<div class="w3-panel w3-border">
  <p><b>RMW_FAKE_HOME</b></p>
Instead of using $HOME, rmw will use $RMW_FAKE_HOME. The configuration
file and default waste directory will be written relative to
$RMW_FAKE_HOME.

<p class="w3-code">
  RMW_FAKE_HOME=$PWD/footest
</p>

While set, using rmw to move any files that reside on the same file
system as *$PWD/footest* will be moved to the waste directories under
*$PWD/footest* and rmw will use the configuration file under
*$PWD/footest*.
</div>

<div class="w3-panel w3-border">
<p><b>RMW_FAKE_YEAR</b></p>

If set to *true* when rmw'ing a file, the year 1999 will be written to
the DeletionDate value in the .trashinfo file (for testing the purge
feature).

<p class="w3-code">
  RMW_FAKE_YEAR=true rmw FILE(s)
</p>

Then running rmw with the purge option will find the items expired and
permanently delete them.
</div>

<div class="w3-panel w3-border">
<p><b>RMW_FAKE_MEDIA_ROOT</b></p>

If set to **true** when rmw-ing a file, relative paths will be written
to the Path key of a .trashinfo file. rmw is faked into believing that
all waste directories are at the top level of a device or removable
medium (see also: <a
href="https://github.com/theimpossibleastronaut/rmw/issues/299">issue
299</a>).

<p class="w3-code">
RMW_FAKE_MEDIA_ROOT=true rmw FILE(s)
</p>
</div>


