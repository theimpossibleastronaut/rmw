#packextrasdir=$(docdir)/src
#SUBDIRS = 
#EXTRA_DIST=rmw.h main.c function_prototypes.h primary_funcs.c Makefile.am Makefile.in
#packextras_DATA=$(EXTRA_DIST)
bin_PROGRAMS=rmw
#AM_CXXFLAGS=$(INTI_CFLAGS)
rmw_SOURCES=str_funcs.c trivial_funcs.c primary_funcs.c main.c
#rmw_LDADD=$(INTI_LIBS)
rmw_CFLAGS = -DSYSCONFDIR="\"${sysconfdir}\""
