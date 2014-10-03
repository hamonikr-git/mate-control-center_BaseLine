ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = po

DISTCHECK_CONFIGURE_FLAGS = \
	--with-avahi \
	--with-libnotify \
	--with-gnome-keyring \
	--with-network-manager \
	--with-telepathy \
	--with-gnutls \
	--with-gcrypt \
	--with-zlib \
	--with-jpeg \
	--enable-debug \
	--enable-http-server \
	--enable-compile-warnings=maximum

noinst_LTLIBRARIES = \
	libeggsmclient.la

libeggsmclient_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	-DEGG_SM_CLIENT_BACKEND_XSMP \
	$(smclient_platform_defines) \
	$(EGG_SMCLIENT_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED) \
	-I$(top_builddir)

libeggsmclient_la_LIBADD = \
	$(EGG_SMCLIENT_LIBS) \
	-lSM -lICE

libeggsmclient_la_SOURCES = \
	server/smclient/eggsmclient.c \
	server/smclient/eggsmclient.h \
	server/smclient/eggsmclient-private.h \
	server/smclient/eggsmclient-xsmp.c \
	server/smclient/eggdesktopfile.c \
	server/smclient/eggdesktopfile.h

if VINO_HAVE_SELF_IFADDRS
VINO_IFADDRS_sources = \
	server/libvncserver/ifaddr/getifaddrs.c \
	server/libvncserver/ifaddr/ifaddrs.h
endif

rfb_headers = \
	server/libvncserver/rfb/rfb.h \
	server/libvncserver/rfb/rfbregion.h \
	server/libvncserver/rfb/rfbproto.h \
	server/libvncserver/rfb/keysym.h

VINO_LIBVNCSERVER_sources = \
	server/libvncserver/auth.c \
	server/libvncserver/corre.c \
	server/libvncserver/cursor.c \
	server/libvncserver/cutpaste.c \
	server/libvncserver/d3des.c \
	server/libvncserver/d3des.h \
	server/libvncserver/hextile.c \
	server/libvncserver/main.c \
	server/libvncserver/rfbregion.c \
	server/libvncserver/rfbserver.c \
	server/libvncserver/rre.c \
	server/libvncserver/sockets.c \
	server/libvncserver/stats.c \
	server/libvncserver/tight.c \
	server/libvncserver/translate.c \
	server/libvncserver/vncauth.c \
	server/libvncserver/zlib.c \
	server/libvncserver/zrle.c \
	server/libvncserver/zrleoutstream.c \
	server/libvncserver/zrleoutstream.h \
	server/libvncserver/zrlepalettehelper.c \
	server/libvncserver/zrlepalettehelper.h \
	server/libvncserver/zrletypes.h \
	$(rfb_headers)

vino_server_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/server/libvncserver \
	-I$(top_srcdir)/server/smclient \
	-I$(top_builddir) \
	-I$(top_builddir)/server \
	$(VINO_SERVER_CFLAGS) \
	$(LIBGCRYPT_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED) \
	-DVINO_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DVINO_ICONDIR=\""$(datadir)/pixmaps\"" \
	-DVINO_CLIENTDIR=\""$(pkgdatadir)"\" \
	-DVINO_UIDIR=\""$(pkgdatadir)"\"

libexec_PROGRAMS = vino-server

vino_server_LDADD = \
	libeggsmclient.la \
	$(VINO_SERVER_LIBS) \
	$(LIBGCRYPT_LIBS) \
	$(JPEG_LIBS) \
	$(LIBUPNP_LIBS) \
	$(X_LIBS) $(XTEST_LIBS) $(XSHM_LIBS) $(XDAMAGE_LIBS) $(ZLIB_LIBS)

if VINO_ENABLE_HTTP_SERVER
VINO_HTTP_sources = \
	server/vino-http.c \
	server/vino-http.h
endif

if VINO_HAVE_TELEPATHY_GLIB
VINO_TELEPATHY_GLIB_sources = \
	server/vino-tube-servers-manager.c \
	server/vino-tube-servers-manager.h \
	server/vino-tube-server.c \
	server/vino-tube-server.h \
	server/vino-status-tube-icon.c \
	server/vino-status-tube-icon.h	\
	server/vino-types.h

clientfiledir = $(datarootdir)/telepathy/clients
dist_clientfile_DATA = server/Vino.client
endif

vino_server_SOURCES = \
	$(marshal_data) \
	$(enum_data) \
	common/vino-dbus.c \
	common/vino-dbus.h \
	server/vino-cursor.c \
	server/vino-cursor.h \
	server/vino-fb.c \
	server/vino-fb.h \
	server/vino-input.c \
	server/vino-input.h \
	server/vino-main.c \
	server/vino-mdns.c \
	server/vino-mdns.h \
	server/vino-prefs.c \
	server/vino-prefs.h \
	server/vino-prompt.c \
	server/vino-prompt.h \
	server/vino-server.c \
	server/vino-server.h \
	server/vino-util.c \
	server/vino-util.h \
	server/vino-status-icon.c \
	server/vino-status-icon.h \
	server/vino-dbus-listener.c \
	server/vino-dbus-listener.h \
	server/vino-dbus-error.c \
	server/vino-dbus-error.h \
	server/vino-upnp.h \
	server/vino-upnp.c \
	$(VINO_IFADDR_sources) \
	$(VINO_LIBVNCSERVER_sources) \
	$(VINO_HTTP_sources) \
	$(VINO_TELEPATHY_GLIB_sources)

if VINO_ENABLE_APP_INDICATOR
vino_server_CPPFLAGS += $(APP_INDICATOR_CFLAGS) -DVINO_ENABLE_APP_INDICATOR
vino_server_LDADD += $(APP_INDICATOR_LIBS)
vino_server_SOURCES += \
	server/vino-app-indicator.c \
	server/vino-app-indicator.h \
	server/vino-tube-app-indicator.c \
	server/vino-tube-app-indicator.h
endif

marshal_data = \
	server/vino-marshal.c \
	server/vino-marshal.h

server/vino-marshal.h: server/vino-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --header --prefix=vino_marshal $< > $@

server/vino-marshal.c: server/vino-marshal.list server/vino-marshal.h
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --header --body --prefix=vino_marshal $< > $@

enum_data = \
	server/vino-enums.c \
	server/vino-enums.h

vino_enum_headers = \
	server/vino-server.h \
	server/vino-prompt.h \
	server/vino-status-icon.h \
	server/vino-status-tube-icon.h

server/vino-enums.c: $(vino_enum_headers) server/vino-enums.h
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--fhead "#include \"vino-enums.h\"\n\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--fprod "#include \"@filename@\"\n" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n" \
		--vhead "  static GType type = 0;\n\n" \
		--vhead "  if (!type)\n  {\n" \
		--vhead "    static const G@Type@Value _@enum_name@_values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n\n" \
		--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n  }\n\n" \
		--vtail "  return type;\n}\n\n" \
		$(vino_enum_headers) > $@

server/vino-enums.h: $(vino_enum_headers)
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--fhead "#ifndef VINO_ENUMS_H_\n" \
		--fhead "#define VINO_ENUMS_H_\n\n" \
		--fhead "#include <glib-object.h>\n\n" \
		--fhead "G_BEGIN_DECLS\n\n" \
		--fprod "/* enumerations from @filename@ */\n" \
		--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n" \
		--vhead "#define VINO_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n" \
		--ftail "#endif /* VINO_ENUMS_H_ */" \
		$(vino_enum_headers) > $@

server/vino-server.desktop.in: server/vino-server.desktop.in.in
	$(AM_V_GEN)$(SED) -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

servicedir = $(datadir)/dbus-1/services
nodist_service_DATA = server/org.freedesktop.Telepathy.Client.Vino.service

server/org.freedesktop.Telepathy.Client.Vino.service: server/org.freedesktop.Telepathy.Client.Vino.service.in
	$(AM_V_GEN)$(SED) -e "s|[@]libexecdir[@]|$(libexecdir)|" $< > $@

bin_PROGRAMS = \
	vino-preferences \
	vino-passwd

vino_preferences_CPPFLAGS = \
	-I$(top_srcdir)/common \
	-I$(top_builddir) \
	-DVINO_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  \
	-DVINO_UIDIR=\""$(pkgdatadir)"\" \
	-DVINO_ICONDIR=\""$(datadir)/pixmaps\"" \
	$(VINO_CAPPLET_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED)

vino_preferences_SOURCES = \
	common/vino-dbus.h \
	common/vino-dbus.c \
	common/vino-keyring.h \
	common/vino-keyring.c \
	capplet/vino-connectivity-info.h \
	capplet/vino-connectivity-info.c \
	capplet/vino-url-webservice.h \
	capplet/vino-url-webservice.c \
	capplet/vino-radio-button.h \
	capplet/vino-radio-button.c \
	capplet/vino-message-box.h \
	capplet/vino-message-box.c \
	capplet/vino-preferences.c

vino_preferences_LDADD = \
	$(VINO_CAPPLET_LIBS) \
	$(X_LIBS)

vino_passwd_CPPFLAGS = \
	-I$(top_builddir) \
	-DVINO_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	$(VINO_TOOLS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED)

vino_passwd_SOURCES = \
	tools/vino-passwd.c

vino_passwd_LDADD = \
	$(VINO_TOOLS_LIBS)

uidir = $(pkgdatadir)
dist_ui_DATA = \
	capplet/vino-preferences.ui \
	capplet/webservices \
	server/vino-prompt.ui

autostartdir = $(sysconfdir)/xdg/autostart
autostart_in_files = server/vino-server.desktop.in
nodist_autostart_DATA = $(autostart_in_files:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_in_files = capplet/vino-preferences.desktop.in
nodist_desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

# GSettings schemas, enum files and conversion file
gsettings_ENUM_NAMESPACE = org.gnome.Vino
gsettings_ENUM_FILES = capplet/vino-preferences.c
gsettings_SCHEMAS = common/org.gnome.Vino.gschema.xml

@GSETTINGS_RULES@

convertdir = $(datadir)/GConf/gsettings
dist_convert_DATA = common/org.gnome.Vino.convert

if MAINTAINER_MODE
dist-hook: dist-changelog
else
dist-hook:
endif

dist-changelog:
	$(AM_V_at)if git --git-dir=$(top_srcdir)/.git --work-tree=$(top_srcdir) \
		log --no-merges --date=short --pretty='tformat:%cd  %an  <%ae>%n%n%s%n%n%b' a65e89b3e25a22ea177581462fb253bc392c789b.. | \
	 $(SED)	-e '/^[12]...-[01].-[0123].  [^<>]*  <[^<>]*>$$/,/^$$/ b' \
		-e '/[^	 ]/,/^[	 ]*$$/ !d' \
		-e 's/^[	 ]*/	/' \
		-e 's/^[	 ]*$$//' >.ChangeLog.tmp; \
	then mv -f .ChangeLog.tmp "$(top_distdir)/ChangeLog"; \
	else rm -f .ChangeLog.tmp; exit 1; fi

dist_noinst_DATA = \
	$(desktop_in_files) \
	\
	$(gsettings_SCHEMAS) \
	\
	docs/remote-desktop.txt \
	docs/debugging.txt \
	docs/TODO \
	\
	server/vino-marshal.list \
	server/vino-server.desktop.in.in \
	server/org.freedesktop.Telepathy.Client.Vino.service.in \
	server/dbus-interface.xml \
	server/libvncserver/tableinit24.c \
	server/libvncserver/tableinittctemplate.c \
	server/libvncserver/tabletrans24template.c \
	server/libvncserver/tabletranstemplate.c \
        server/libvncserver/tableinitcmtemplate.c \
        server/libvncserver/zrleencodetemplate.c \
	\
	ChangeLog.pre-git \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	vino.doap

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = \
	$(nodist_desktop_DATA) \
	$(nodist_service_DATA) \
	$(marshal_data) \
	$(enum_data) \
	server/vino-server.desktop \
	server/vino-server.desktop.in

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache

MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)

.PHONY: dist-changelog
