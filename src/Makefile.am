#packextrasdir=$(docdir)/src
#SUBDIRS =
#rmw_includedir=functions
#EXTRA_DIST=rmw.h main.c function_prototypes.h primary_funcs.c Makefile.am Makefile.in
#packextras_DATA=$(EXTRA_DIST)
bin_PROGRAMS=rmw
#AM_CXXFLAGS=$(INTI_CFLAGS)
rmw_SOURCES=		\
primary_funcs.c		\
main.c 			\
./functions/restore_rmw.c \
./functions/config_rmw.c	\
./functions/trivial_rmw.c \
./functions/strings_rmw.c \
./functions/purging_rmw.c

#rmw_LDADD=$(INTI_LIBS)
rmw_CFLAGS =  -std=gnu99 -Wall -I./functions  -DSYSCONFDIR=\"${sysconfdir}\"
