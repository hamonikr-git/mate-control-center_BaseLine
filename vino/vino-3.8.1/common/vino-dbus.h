/*
 * Copyright © 2010 Codethink Limited
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __vino_dbus_h__
#define __vino_dbus_h__

#include <gio/gio.h>

extern const GDBusInterfaceInfo org_gnome_VinoScreen_interface;
#define ORG_GNOME_VINO_SCREEN_INTERFACE_NAME    "org.gnome.VinoScreen"
#define ORG_GNOME_VINO_BUS_NAME                 "org.gnome.Vino"
#define ORG_GNOME_VINO_SCREEN_PATH_PREFIX       "/org/gnome/vino/screens/"
#define ORG_GNOME_VINO_SCREEN_INTERFACE \
  ((GDBusInterfaceInfo *) &org_gnome_VinoScreen_interface)

#endif /* __vino_dbus_h__ */
