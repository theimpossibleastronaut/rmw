---
title: Code Testing
layout: default
---
<ul>
  <li><a href="#general_testing">General Testing</a></li>
  <li><a href="#testing_purge">Testing the purge feature</a></li>
  <li><a href="#profiling">Profiling</a></li>
</ul>

<h2 id="general_testing">General Testing</h2>

If you're writing a patch, using <code class="w3-codespan">make
check</code> will cover most of rmw's operations. If there's a test
missing, please open a ticket.

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
  make clean<br />
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
