LINGUAS = de pt_BR es pl fi ne nl

install-data-local:
		$(mkinstalldirs) $(DESTDIR)$(mandir)/man1; \
		$(INSTALL_DATA) $(PACKAGE_NAME).1 $(DESTDIR)$(mandir)/man1; \
		for lang in $(LINGUAS); do \
			file=$(srcdir)/$$lang/$(PACKAGE_NAME).1; \
			dest=$(DESTDIR)$(mandir)/$$lang/man1; \
			dest=`echo $$dest | sed '$(program_transform_name)'`; \
			$(mkinstalldirs) $$dest; \
			$(INSTALL_DATA) $$file $$dest; \
		done;

uninstall-local:
	if test -f $(DESTDIR)$(mandir)/man1/$(PACKAGE_NAME).1; then \
			rm -f $(DESTDIR)$(mandir)/man1/$(PACKAGE_NAME).1; \
	fi; \
	for lang in $(LINGUAS); do \
		dest=$(DESTDIR)$(mandir)/$$lang/man1/$(PACKAGE).1; \
		dest=`echo $$dest | sed '$(program_transform_name)'`; \
		if test -f $$dest; then \
			rm -f $$dest; \
		fi \
	done;
