test_scripts = basic.sh purging.sh

EXTRA_DIST=	\
	README	\
	$(test_scripts)

test:
	vars=$(builddir)/VARS;	\
	for script in $(test_scripts); do	\
		if ! test -f $(builddir)/$$script; then	\
			cp $(srcdir)/$$script $(builddir);	\
		fi	\
	done;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home > $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/etc/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(builddir)/basic.sh $$vars;	\
	$(builddir)/purging.sh $$vars;
