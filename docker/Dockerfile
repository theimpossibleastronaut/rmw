FROM andy5995/rmw-build-env:bullseye
USER root
RUN apt-get update \
  && apt-get install -y git

WORKDIR /rmw
USER rmw
RUN git clone --depth 1 https://github.com/theimpossibleastronaut/rmw src
WORKDIR /rmw/src
RUN meson ../builddir \
  --prefix=/usr \
  -Dc_args=-static \
  -Dc_link_args=-static

WORKDIR /rmw/builddir
RUN ninja
USER root
RUN ninja install

FROM debian:bullseye-slim
COPY --from=0 /usr/bin/rmw /usr/bin
COPY --from=0 /usr/share/doc/rmw /usr/share/doc/rmw
COPY --from=0 /usr/share/locale /usr/share/locale

CMD ["/bin/bash"]
