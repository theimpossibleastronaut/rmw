FROM andy5995/rmw:build-env-alpine

RUN git clone --depth 1 https://github.com/theimpossibleastronaut/rmw \
    && cd /rmw \
    && meson setup builddir --buildtype=release -Db_sanitize=none --prefix=/usr \
    && cd /rmw/builddir \
    && ninja -v \
    && meson test -v \
    && DESTDIR=/tmp/rmw ninja install

FROM alpine
COPY --from=0 /tmp/rmw/ /
RUN apk add \
      libmenuw \
      mandoc \
      musl-fts \
      ncurses

CMD ["/bin/sh","-l"]
