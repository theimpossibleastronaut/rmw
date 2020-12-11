---
title: Code Testing
layout: default
---
<ul>
  <li><a href="#general_testing">General Testing</a></li>
  <li><a href="#testing_home">Specify different home directory</a></li>
  <li><a href="#testing_purge">Testing the purge feature</a></li>
  <li><a href="#profiling">Profiling</a></li>
  <li><a href="#create_test">How to Create a Unit Test</a></li>
</ul>

<h2 id="general_testing">General Testing</h2>

If you're writing a patch, using <code class="w3-codespan">make
check</code> will cover most of rmw's operations. If there's a test
missing, please open a ticket.

If [valgrind](https://www.valgrind.org/) is installed, the check will
include some tests for resource leaks. To bypass using `valgrind`, use
<code class="w3-codespan">USE_VALGRIND=0 make check</code>

<h2 id="testing_home">Specify different home directory</h2>

As of v0.7.03, you can provide a "fake" home directory by setting the
environmental variable RMWTEST_HOME.

<h2 id="testing_purge">Testing the purge feature</h2>

Setting the RMWTRASH environmental variable at run-time to "fake-year"
will write the year 1999 to the DeletionDate string in the .trashinfo
file.

<p class="w3-code">
  RMWTRASH=fake-year rmw some.txt temp.asc files.doc
</p>

Then run rmw with the purge option

<p class="w3-code">
  rmw -fg
</p>

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
class="w3-codespan">gmon.out</code>. Now run <code

class="w3-codespan">gprof ./rmw gmon.out</code> to view the results.
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

