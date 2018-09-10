
# Install the English man file and include it when `make dist` is used
dist_man_MANS=rmw.1

LINGUAS = de es fi ne nl pl pt_BR

EXTRA_DIST = $(LINGUAS)

install-data-local:
		for lang in $(LINGUAS); do \
			file=$(srcdir)/$$lang/$(PACKAGE_NAME).1; \
			dest=$(DESTDIR)$(mandir)/$$lang/man1; \
			dest=`echo $$dest | sed '$(program_transform_name)'`; \
			$(mkinstalldirs) $$dest; \
			$(INSTALL_DATA) $$file $$dest; \
		done;

uninstall-local:
	for lang in $(LINGUAS); do \
		dest=$(DESTDIR)$(mandir)/$$lang/man1/$(PACKAGE).1; \
		dest=`echo $$dest | sed '$(program_transform_name)'`; \
		if test -f $$dest; then \
			rm -f $$dest; \
		fi \
	done;
