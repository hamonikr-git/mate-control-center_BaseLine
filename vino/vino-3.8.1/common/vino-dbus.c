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

#include "vino-dbus.h"

static const GDBusPropertyInfo org_gnome_VinoScreen_ExternalHost = {
  -1, "ExternalHost", "s", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo org_gnome_VinoScreen_ExternalPort = {
  -1, "ExternalPort", "q", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo org_gnome_VinoScreen_Host = {
  -1, "Host", "s", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo org_gnome_VinoScreen_Port = {
  -1, "Port", "q", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo org_gnome_VinoScreen_AvahiHost = {
  -1, "AvahiHost", "s", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo org_gnome_VinoScreen_Connected = {
  -1, "Connected", "b", G_DBUS_PROPERTY_INFO_FLAGS_READABLE
};

static const GDBusPropertyInfo* const org_gnome_VinoScreen_properties[] = {
  &org_gnome_VinoScreen_ExternalHost,
  &org_gnome_VinoScreen_ExternalPort,
  &org_gnome_VinoScreen_AvahiHost,
  &org_gnome_VinoScreen_Host,
  &org_gnome_VinoScreen_Port,
  &org_gnome_VinoScreen_Connected,
  NULL
};

const GDBusInterfaceInfo org_gnome_VinoScreen_interface = {
  -1,
  "org.gnome.VinoScreen",
  (GDBusMethodInfo **)     NULL,
  (GDBusSignalInfo **)     NULL,
  (GDBusPropertyInfo **)   org_gnome_VinoScreen_properties,
  (GDBusAnnotationInfo **) NULL
};
