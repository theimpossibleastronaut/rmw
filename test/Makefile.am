test_scripts = basic.sh purging.sh

EXTRA_DIST=	\
	README	\
	COMMON \
	$(test_scripts)

test-basic:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/etc/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(srcdir)/basic.sh $$vars;

test-purging:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/etc/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(srcdir)/purging.sh $$vars;

testclean:
	for file in "VARS" "tmp-home"; do	\
		if test -e $$file; then	\
			rm -rf $$file;	\
		fi	\
	done;

