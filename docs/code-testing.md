---
title: Code Testing
layout: default
---
<ul>
  <li><a href="#general_testing">General Testing</a></li>
  <li><a href="#profiling">Profiling</a></li>
  <li><a href="#create_test">How to Create a Unit Test</a></li>
  <li><a href="#env_vars">Environmental Variables</a></li>
</ul>

<h2 id="general_testing">General Testing</h2>

If you're writing a patch, using <code class="w3-codespan">make
check</code> will cover most of rmw's operations. If there's a test
missing, please open a ticket.

If [valgrind](https://www.valgrind.org/) is installed, the check will
include some tests for resource leaks. To bypass using `valgrind`, use
<code class="w3-codespan">USE_VALGRIND=0 make check</code>

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

<h2 id="create_test">How to Create a Unit Test</h2>

[This
example](https://github.com/theimpossibleastronaut/rmw/commit/edaf560929e8589bac8874b93ae3520962ffab39)
shows how to create a unit test after adding a new function. In
strings_rmw.c I've added the function
[bufchk_len()](https://github.com/theimpossibleastronaut/rmw/commit/edaf560929e8589bac8874b93ae3520962ffab39#diff-20cfff9d32e70348c58a461184f4070eR123).
The test I created for it is
[buffer_overrun.c](https://github.com/theimpossibleastronaut/rmw/commit/edaf560929e8589bac8874b93ae3520962ffab39#diff-14a4f62c9e948bbebcfc09c12e01f3ae);
then I've added the test to
[test/Makefile.am](https://github.com/theimpossibleastronaut/rmw/commit/edaf560929e8589bac8874b93ae3520962ffab39#diff-7d1a3afeff4f7c00c95d6be6f2847e6e)
and re-ran 'automake'.

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


