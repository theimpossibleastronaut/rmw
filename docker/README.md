# Docker

You may find the Docker images useful (or you may not).

## [andy5995/rmw-build-env](https://hub.docker.com/repository/docker/andy5995/rmw-build-env)

Image with the rmw build environment (meson, ncurses, and gettext) on
Debian bullseye.

One use-case for this image is mounting your local rmw source
directory and running tests inside the container (not really required
for testing in most cases because the rmw tests already run in a
sandbox).

Example (from the src_root/docker directory):

    docker compose up

That will mount the source directory at '/rmw/src', build rmw based on
your local changes, and run the tests.

To get verbose test output, change the value of RMW_V to `-v` in
docker-compose.yml.

After you run `docker compose`, there will be a stopped container, which you
can see by entering `docker ps -a`. You can remove this container anytime by
entering:

    docker container rm rmw-test-current

## [andy5995/rmw](https://hub.docker.com/repository/docker/andy5995/rmw)

Image contains a build of rmw in '/usr/bin'. You can try it out inside the
container if you like.
