#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME=gconf-editor

REQUIRED_AUTOMAKE_VERSION=1.9
REQUIRED_INTLTOOL_VERSION=0.40.0

if ! test -f $srcdir/src/gconf-editor-application.c; then
  echo "**Error**: Directory '$srcdir' does not look like the gconf-edtior source directory"
  exit 1
fi

which gnome-autogen.sh || {
    echo "You need to install gnome-common from Gnome SVN"
    exit 1
}

. gnome-autogen.sh
