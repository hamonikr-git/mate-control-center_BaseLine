langtoolsdir = ${datadir}/language-tools

install-data-hook:
	if test '!' -d $(DESTDIR)$(langtoolsdir); then \
		$(mkinstalldirs) $(DESTDIR)$(langtoolsdir); \
		chmod 755 $(DESTDIR)$(langtoolsdir); \
	fi

	$(INSTALL_SCRIPT)  language-options          $(DESTDIR)$(langtoolsdir)/language-options
	$(INSTALL_SCRIPT)  language-validate         $(DESTDIR)$(langtoolsdir)/language-validate
	$(INSTALL_SCRIPT)  language2locale           $(DESTDIR)$(langtoolsdir)/language2locale
	$(INSTALL_SCRIPT)  locale2papersize          $(DESTDIR)$(langtoolsdir)/locale2papersize
	$(INSTALL_DATA)    main-countries            $(DESTDIR)$(langtoolsdir)/main-countries
	$(INSTALL_SCRIPT)  save-to-pam-env           $(DESTDIR)$(langtoolsdir)/save-to-pam-env
	$(INSTALL_SCRIPT)  set-language-helper       $(DESTDIR)$(langtoolsdir)/set-language-helper
	$(INSTALL_SCRIPT)  update-langlist           $(DESTDIR)$(langtoolsdir)/update-langlist

