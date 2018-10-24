test_scripts =	\
	basic.sh	\
	purging.sh	\
	restore.sh

EXTRA_DIST=	\
	README	\
	COMMON \
	$(test_scripts)

# If anything needs to change for the target tests, just edit the first
# one and then copy the info to the others. Change just the script name
# on the last line of each target.

test-all:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	for script in $(test_scripts); do \
		$(srcdir)/$$script $$vars;	\
	done;

test-basic:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(srcdir)/basic.sh $$vars;

test-purging:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(srcdir)/purging.sh $$vars;

test-restore:
	vars=$(builddir)/VARS;	\
	echo "# This file was automatically generated. DO NOT EDIT" > $$vars;	\
	echo "HOME=${PWD}"/$(builddir)/tmp-home >> $$vars;	\
	echo "TESTS_DIR=${PWD}"/$(srcdir) >> $$vars;	\
	echo "CONFIG=${PWD}"/$(top_srcdir)/rmwrc >> $$vars;	\
	echo "BIN_DIR=${PWD}"/$(top_builddir)/src >> $$vars;	\
	$(srcdir)/restore.sh $$vars;

testclean:
	for file in "VARS" "tmp-home"; do	\
		if test -e $$file; then	\
			rm -rf $$file;	\
		fi	\
	done;

