# Docker

You may find the Docker images useful (or you may not).

## [andy5995/rmw](https://hub.docker.com/repository/docker/andy5995/rmw)

Image contains a build of rmw in '/usr/bin'. You can try it out inside the
container if you like.

    docker pull andy5995/rmw:latest (latest git revision)
    docker pull andy5995/rmw:0.9.0 (latest release)

## [andy5995/rmw-build-env](https://hub.docker.com/repository/docker/andy5995/rmw-build-env)

Image with the rmw build environment (meson, ncurses, and gettext) on
Debian bullseye.

You can build and test rmw in a docker container:

    docker run -it --rm --entrypoint /rmw/entrypoint-test.sh -v `pwd`:/rmw/src andy5995/rmw-build-env:bullseye

That will mount the source directory at '/rmw/src', build rmw based on
your local changes, and run the tests.
