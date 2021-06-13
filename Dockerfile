FROM gcc
COPY . /usr/src/rmw
WORKDIR /usr/src/_build_rmw
RUN ../rmw/configure
RUN make check
RUN make install
RUN make distclean
