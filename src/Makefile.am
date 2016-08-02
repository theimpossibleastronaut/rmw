#packextrasdir=$(docdir)/src
#SUBDIRS = 
#EXTRA_DIST=rmw.c funcs.c funcs02.c Makefile.am Makefile.in
#packextras_DATA=$(EXTRA_DIST)
bin_PROGRAMS=rmw
#AM_CXXFLAGS=$(INTI_CFLAGS)
rmw_SOURCES=main.c str_funcs.c trivial_funcs.c primary_funcs.c
#rmw_LDADD=$(INTI_LIBS)
rmw_CFLAGS = -DSYSCONFDIR="\"${sysconfdir}\""
