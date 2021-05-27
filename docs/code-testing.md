---
title: Code Testing
layout: default
---
<ul>
  <li><a href="#general_testing">General Testing</a></li>
  <li><a href="#profiling">Profiling</a></li>
  <li><a href="#env_vars">Environmental Variables</a></li>
</ul>

<h2 id="general_testing">General Testing</h2>

If you're writing a patch, using <code class="w3-codespan">make
check</code> will cover most of rmw's operations. If there's a test
missing, please open a ticket.

To enable tests with [valgrind](https://www.valgrind.org/), give the
<code class="w3-codespan">--enable-valgrind</code> option to the
configure script.

<!-- This section probably would be better on a separate "Debugging" page (not yet created) -->
<h2 id="profiling">Profiling</h2>

To get [a profile of rmw](/profile.example.txt) using [GNU
gprof](https://sourceware.org/binutils/docs/gprof/), first you'll need
to rebuild rmw with different [compiler
flags](https://sourceware.org/binutils/docs/gprof/Compiling.html#Compiling).
The easiest way to do that is

<p class="w3-code">
  make distclean<br />
  ../configure --enable-debug=profile<br />
  make
</p>

Then run rmw with any parameters you like.

<p class="w3-code">
  ex: ./rmw -l
</p>

That will produce a file called <code
class="w3-codespan">gmon.out</code>. To view the results, run
<p class="w3-code">gprof ./rmw gmon.out</p>
(To learn about runtime options for gprof, check the documentation on
its website.)

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


