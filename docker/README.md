# Docker

You may find the Docker images useful (or you may not).

## [andy5995/rmw-build-env](https://hub.docker.com/repository/docker/andy5995/rmw-build-env)

Image with the rmw build environment (meson, ncurses, and gettext) on
Debian bullseye.

One use-case for this image is mounting your local rmw source
directory and running tests inside the container (not really required
for testing in most cases because the rmw tests already run in a
sandbox).

Example (if your present working directory is the rmw source directory):

    docker run --rm --entrypoint /rmw/entrypoint-test.sh  -v `pwd`:/rmw/src andy5995/rmw-build-env:bullseye

That will mount the source directory at '/rmw/src', build rmw based on
your local changes, and run the tests. To test with valgrind, add `-e
USE_VALGRIND=true` to the docker 'run' arguments.

Specifying the entrypoint isn't mandatory; if omitted, you'll get a prompt.

## [andy5995/rmw](https://hub.docker.com/repository/docker/andy5995/rmw)

Image contains a static build of rmw in '/usr/bin'. You can try it out
inside the container if you like.
