AC_PREREQ([2.64])
AC_INIT([Vino], [3.8.1], [https://bugzilla.gnome.org/enter_bug.cgi?product=vino], [vino], [https://live.gnome.org/Vino])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([server/vino-fb.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11.1 -Wall check-news dist-xz no-define no-dist-gzip nostdinc subdir-objects])
AM_SILENT_RULES([yes])

AC_PROG_CC
AM_PROG_CC_C_O

# Check whether to enable HTTP server support
AC_ARG_ENABLE([http-server],
  [AS_HELP_STRING([--disable-http-server],
   [build with an HTTP server for serving a Java applet client @<:@default=yes@:>@])],
   [],
   [enable_http_server=yes])

AS_IF([test "x$enable_http_server" = "xyes"],
  [AC_DEFINE([VINO_ENABLE_HTTP_SERVER], [], [Enable HTTP server support])])

AM_CONDITIONAL([VINO_ENABLE_HTTP_SERVER], [test "x$enable_http_server" = "xyes"])

# GNOME common
AM_MAINTAINER_MODE([enable])
GNOME_MAINTAINER_MODE_DEFINES
GNOME_COMPILE_WARNINGS([maximum])
GNOME_DEBUG_CHECK

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

PKG_PROG_PKG_CONFIG([0.16])

AC_PROG_SED

AC_MSG_CHECKING([for glib-genmarshal script])
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AS_IF([test "x$GLIB_GENMARSHAL" = "x"],
  [AC_MSG_ERROR([glib-genmarshal not listed in glib-2.0 pkg-config file])],
  [AC_SUBST([GLIB_GENMARSHAL])
    AC_MSG_RESULT([$GLIB_GENMARSHAL])])

AC_MSG_CHECKING([for glib-mkenums script])
GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AS_IF([test "x$GLIB_MKENUMS" = "x"],
  [AC_MSG_ERROR([glib-mkenums not listed in glib-2.0 pkg-config file])],
  [AC_SUBST([GLIB_MKENUMS])
    AC_MSG_RESULT([$GLIB_MKENUMS])])

# Check for telepathy-glib
TELEPATHY_GLIB_DEPS="dbus-glib-1 telepathy-glib >= 0.18.0"

AC_ARG_WITH([telepathy],
  [AS_HELP_STRING([--without-telepathy],
    [ignore the presence of telepathy and disable it])])

AS_IF([test "x$with_telepathy" != "xno"],
  [PKG_CHECK_EXISTS([$TELEPATHY_GLIB_DEPS],
    [have_telepathy=yes],
    [have_telepathy=no])],
  [have_telepathy=no])

AS_IF([test "x$have_telepathy" = "xyes"],
  [AC_DEFINE([VINO_HAVE_TELEPATHY_GLIB], [], [Build with telepathy support])],
  [TELEPATHY_GLIB_DEPS=""
    AS_IF([test "x$with_telepathy" = "xyes"],
    [AC_MSG_ERROR([telepathy support requested but not found])])])

AM_CONDITIONAL([VINO_HAVE_TELEPATHY_GLIB], [test "x$have_telepathy" = "xyes"])

# Check for NetworkManager
NETWORKMANAGER_DEPS="NetworkManager >= 0.7"

AC_ARG_WITH([network-manager],
  [AS_HELP_STRING([--without-network-manager],
    [ignore the presence of NetworkManager and disable it])])

AS_IF([test "x$with_network_manager" != "xno"],
  [PKG_CHECK_EXISTS([$NETWORKMANAGER_DEPS],
    [have_network_manager=yes],
    [have_network_manager=no])],
  [have_network_manager=no])

AS_IF([test "x$have_network_manager" = "xyes"],
  [AC_DEFINE([VINO_HAVE_NETWORKMANAGER], [], [Build with NetworkManager support])],
  [NETWORKMANAGER_DEPS=""
    AS_IF([test "x$with_network_manager" = "xyes"],
    [AC_MSG_ERROR([NetworkManager support requested but not found])])])

# Check for secret
SECRET_DEPS="libsecret-1"

AC_ARG_WITH([secret],
  [AS_HELP_STRING([--without-secret],
    [ignore the presence of secret and disable it])])

AS_IF([test "x$with_secret" != "xno"],
  [PKG_CHECK_EXISTS([$SECRET_DEPS],
    [have_secret=yes],
    [have_secret=no])],
  [have_secret=no])

AS_IF([test "x$have_secret" != "xno"],
  [AC_DEFINE([VINO_HAVE_SECRET], [], [Build with secret support])],
  [SECRET_DEPS=""
    AS_IF([test "x$with_secret" = "xyes"],
    [AC_MSG_ERROR([secret support requested but not found])])])

# Check for X11
AC_PATH_XTRA

AS_IF([test "x$no_x" = "xyes"],
  [AC_MSG_ERROR([X development libraries not found])],
  [AC_SUBST([X_LIBS], ["$X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"])])

# Check for gnutls
GNUTLS_DEPS="gnutls >= 2.2.0"

AC_ARG_WITH([gnutls],
  [AS_HELP_STRING([--without-gnutls],
    [ignore gnutls support and disable it])])

AS_IF([test "x$with_gnutls" != "xno"],
  [PKG_CHECK_EXISTS([$GNUTLS_DEPS],
    [have_gnutls=yes],
    [have_gnutls=no])],
  [have_gnutls=no
    AC_MSG_WARN([gnutls was not found. You may want to get it from ftp://ftp.gnutls.org/pub/gnutls/])])

AS_IF([test "x$have_gnutls" = "xyes"],
  [AC_DEFINE([VINO_HAVE_GNUTLS], [], [Build with gnutls support])],
  [GNUTLS_DEPS=""
    AS_IF([test "x$with_gnutls" = "xyes"],
    [AC_MSG_ERROR([gnutls support was requested but not found])])])

# Check for gcrypt
GCRYPT_VERSION=1.1.90

AC_ARG_WITH([gcrypt],
  [AS_HELP_STRING([--without-gcrypt],
    [ignore gcrypt support and disable it])])

AS_IF([test "x$with_gcrypt" != "xno"],
  [AM_PATH_LIBGCRYPT([$GCRYPT_VERSION],
    [have_gcrypt=yes],
    [have_gcrypt=no])],
  [have_gcrypt=no
    AC_MSG_WARN([gcrypt was not found. You may want to get it from ftp://ftp.gnupg.org/pub/gcrypt/alpha/libgcrypt/])])

AS_IF([test "x$have_gcrypt" = "xyes"],
  [AC_DEFINE([VINO_HAVE_GCRYPT], [], [Build with gcrypt support])],
  [AS_IF([test "x$with_gcrypt" = "xyes"],
    [AC_MSG_ERROR([gcrypt support requested but not found])])])

# Check for Avahi
AVAHI_DEPS="avahi-client >= 0.6 avahi-glib >= 0.6"

AC_ARG_WITH([avahi],
  [AS_HELP_STRING([--without-avahi],
    [ignore avahi support and disable it])])

AS_IF([test "x$with_avahi" != "xno"],
  [PKG_CHECK_EXISTS([$AVAHI_DEPS],
    [have_avahi=yes],
    [have_avahi=no])],
  [have_avahi=no])

AS_IF([test "x$have_avahi" = "xyes"],
  [AC_DEFINE([VINO_HAVE_AVAHI], [], [Build with avahi support])],
  [AVAHI_DEPS=""
    AS_IF([test "x$with_avahi" = "xyes"],
    [AC_MSG_ERROR([avahi support requested but not found])])])

# Check for zlib
# TODO: Use pkg-config file installed by newer zlib releases
AC_ARG_WITH([zlib],
  [AS_HELP_STRING([--without-zlib],
    [ignore zlib support and disable it])])

AS_IF([test "x$with_zlib" != "xno"],
  [AC_CHECK_HEADER([zlib.h],
    [AC_CHECK_LIB([z], [deflate], [have_zlib=yes], [have_zlib=no])],
    [have_zlib=no])],
  [have_zlib=no])

AS_IF([test "x$have_zlib" = "xyes"],
  [AC_DEFINE([VINO_HAVE_ZLIB], [], [Build with zlib support])
    AC_SUBST([ZLIB_LIBS], [-lz])],
  [AS_IF([test "x$with_zlib" = "xyes"],
    [AC_MSG_ERROR([zlib support requested but not found])])])

# Check for jpeg, with deflate support
AC_ARG_WITH([jpeg],
  [AS_HELP_STRING([--without-jpeg],
    [ignore jpeg support and disable it])])

AS_IF([test "x$with_jpeg" != "xno"],
  [AC_CHECK_HEADER([jpeglib.h],
    [AC_CHECK_LIB([jpeg], [jpeg_CreateCompress], [have_jpeg=yes], [have_jpeg=no])],
    [have_jpeg=no])],
  [have_jpeg=no])

AS_IF([test "x$have_jpeg" = "xyes"],
  [AC_DEFINE([VINO_HAVE_JPEG], [], [Build with jpeg support])
    AC_SUBST([JPEG_LIBS], [-ljpeg])],
  [AS_IF([test "x$with_jpeg" = "xyes"],
    [AC_MSG_ERROR([jpeg support requested but not found])])])

# Internationalization
AC_SUBST([GETTEXT_PACKAGE],[$PACKAGE_TARNAME])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [gettext package])
IT_PROG_INTLTOOL([0.50.0])

# Check for XDAMAGE extension
AC_CHECK_HEADER([X11/extensions/Xdamage.h],
  [AC_CHECK_LIB([Xdamage], [XDamageQueryExtension],
    [AC_DEFINE([VINO_HAVE_XDAMAGE], [], [Defined if the DAMAGE X extension is present])
      AC_SUBST([XDAMAGE_LIBS], ["-lXdamage -lXfixes"])],
    [], [$X_LIBS])],
  [], [#include <X11/Xlib.h>])

# Check for MIT-SHM extension
AC_CHECK_HEADER([X11/extensions/XShm.h],
  [AC_CHECK_LIB([Xext], [XShmQueryExtension],
    [AC_DEFINE([VINO_HAVE_XSHM], [], [Defined if the MIT-SHM X extension is present])
      AC_SUBST([XSHM_LIBS], [-lXext])],
    [], [$X_LIBS])],
  [], [#include <X11/Xlib.h>])

# Check for XTest extension
AC_CHECK_HEADER([X11/extensions/XTest.h],
  [AC_CHECK_LIB([Xtst], [XTestQueryExtension],
    [AC_DEFINE([VINO_HAVE_XTEST], [], [Defined if the XTEST X extension is present])
      AC_SUBST([XTEST_LIBS], [-lXtst])],
    [], [$X_LIBS])])

# Check for XKB extension
AC_CHECK_HEADER([X11/XKBlib.h],
  [AC_CHECK_LIB([X11], [XkbQueryExtension],
    [AC_DEFINE([VINO_HAVE_XKB], [], [Defined if the XKB X extension is present])],
  [], [$X_LIBS])])

AC_CHECK_HEADERS([netinet/in.h sys/time.h fcntl.h unistd.h sys/socket.h ifaddrs.h])

# Check for getifaddrs
AM_CONDITIONAL([VINO_HAVE_SELF_IFADDRS], [test "x$ac_cv_header_ifaddrs_h" != "xyes"])

# RFC2553 introduce sockaddr_storage as ifa_addr member in ifaddrs structure
# Not all distros follow this.
AS_IF([test "x$ac_cv_header_ifaddrs_h" = "xyes"],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <net/if.h>
    #include <ifaddrs.h>
    ]], [[
    struct ifaddrs *myaddrs;
    getifaddrs (&myaddrs);
    if (myaddrs->ifa_addr->ss_family == AF_INET) {
    }
    ]])],
    [have_sockaddr_storage=yes],
    [have_sockaddr_storage=no])],
  [AS_IF([test "x$have_sockaddr_storage" = "xyes"],
    [AC_DEFINE([RFC2553], [], [Define if RFC2553 is followed])])])

AC_C_BIGENDIAN
AC_C_INLINE

AC_CHECK_LIB([resolv], [herror])
AC_SEARCH_LIBS([strerror],[cposix])
AC_CHECK_FUNCS([gettimeofday])

# Check for IPv6 support
AC_MSG_CHECKING([checking for IPv6 support])
AC_ARG_ENABLE([ipv6],
  [AS_HELP_STRING([--disable-ipv6],
    [ignore the presence of IPv6 support and disable it])],
    [],
    [enable_ipv6=yes])

AS_IF([test "x$enable_ipv6" != "xno"],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <sys/types.h>
    #include <sys/socket.h>
    ]], [[
      socket(AF_INET6, SOCK_STREAM, 0);
    ]])],
    [have_ipv6=yes],
    [have_ipv6=no])])

AS_IF([test "x$have_ipv6" = "xyes"],
  [AC_DEFINE([VINO_ENABLE_IPV6], [], [Define to enable IPv6 support])],
  [AS_IF([test "x$enable_ipv6" = "xyes"],
    [AC_MSG_ERROR([IPv6 support requested but not found])])])
AC_MSG_RESULT([$have_ipv6])

dnl ----------------------------------------------------------------------------
dnl
dnl Allow use of system miniupnp library

LIBUPNP_LIBS="-lminiupnpc"
AC_SUBST(LIBUPNP_LIBS)

# GNOME libraries
GTK_VERSION=3.0.0
GLIB_VERSION=2.17.0
GIO_VERSION=2.26
SOUP_VERSION=2.24.0

PKG_CHECK_MODULES([VINO_SERVER], [glib-2.0 >= $GLIB_VERSION gio-unix-2.0 gtk+-x11-3.0 >= $GTK_VERSION $TELEPATHY_GLIB_DEPS libnotify >= 0.7.0 $SECRET_DEPS $NETWORKMANAGER_DEPS $GNUTLS_DEPS $AVAHI_DEPS])

PKG_CHECK_MODULES([VINO_CAPPLET], [glib-2.0 >= $GLIB_VERSION gio-2.0 >= $GIO_VERSION gtk+-3.0 >= $GTK_VERSION libsoup-2.4 >= $SOUP_VERSION $SECRET_DEPS])

PKG_CHECK_MODULES([VINO_TOOLS], [glib-2.0 >= $GLIB_VERSION gio-2.0 >= $GIO_VERSION gobject-2.0 >= $GLIB_VERSION $SECRET_DEPS])

# EGG stuff
PKG_CHECK_MODULES([EGG_SMCLIENT], [gtk+-3.0])

GLIB_GSETTINGS

AC_CONFIG_FILES([
Makefile
capplet/vino-preferences.desktop.in
po/Makefile.in
])

AC_OUTPUT
