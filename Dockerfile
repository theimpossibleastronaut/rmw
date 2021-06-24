FROM andy5995/meson
COPY . /usr/src/rmw
WORKDIR /usr/src/rmw
RUN meson builddir
WORKDIR /usr/src/rmw/builddir
RUN ninja -v
RUN ninja dist
RUN meson configure -Dnls=false
RUN meson test --setup=valgrind
