FROM andy5995/rmw-build-env:alpine

RUN git clone --depth 1 https://github.com/theimpossibleastronaut/rmw
WORKDIR /rmw
RUN LDFLAGS=-lfts meson setup builddir --buildtype=release -Db_sanitize=none --prefix=/usr
WORKDIR /rmw/builddir
RUN ninja -v && meson test -v
RUN DESTDIR=/tmp/rmw ninja install

FROM alpine
COPY --from=0 /tmp/rmw/ /
RUN apk add \
      mandoc \
      musl-fts \
      ncurses

CMD ["/bin/sh","-l"]
