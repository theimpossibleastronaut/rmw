FROM andy5995/meson
COPY . /usr/src/rmw
WORKDIR /usr/src/rmw
RUN sh .github/scripts/all_tests.sh
