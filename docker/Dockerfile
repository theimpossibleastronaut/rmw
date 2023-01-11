FROM andy5995/rmw-build-env:bullseye
USER root
RUN apt-get update \
  && apt-get install -y git

WORKDIR /rmw
USER rmw
RUN git clone --depth 1 https://github.com/theimpossibleastronaut/rmw src
WORKDIR /rmw/src
RUN meson setup ../builddir \
  --prefix=/usr

WORKDIR /rmw/builddir
RUN ninja
RUN DESTDIR=/tmp/rmw ninja install

FROM debian:bullseye-slim
COPY --from=0 /tmp/rmw/ /
RUN apt-get update && apt-get -y install \
      libncursesw6  \
      libasan6  \
      libubsan1 \
      man \
    && rm -rf /var/lib/apt/lists

CMD ["/bin/bash","-l"]
