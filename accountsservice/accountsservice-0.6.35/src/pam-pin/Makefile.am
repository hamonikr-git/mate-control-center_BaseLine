securitydir = $(libdir)/security

security_LTLIBRARIES = pam_pin.la

pam_pin_la_SOURCES = pam-module.c
pam_pin_la_LDFLAGS = -module -avoid-version
pam_pin_la_LIBADD = $(LIBGNUTLS_LIBS) $(LIBGCRYPT_LIBS) $(GIO_LIBS)
pam_pin_la_CFLAGS = \
	-DLOCALSTATEDIR=\""$(localstatedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DUSERDIR=\"$(localstatedir)/lib/AccountsService/users\" \
	-DPASSWDDIR=\"$(localstatedir)/lib/AccountsService/encrypted-password\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	$(LIBGNUTLS_CFLAGS) \
	$(LIBGCRYPT_CFLAGS) \
	$(GIO_CFLAGS) \
	$(WARN_CFLAGS)
