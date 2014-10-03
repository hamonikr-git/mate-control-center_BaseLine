/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GCONF_UTIL_H__
#define __GCONF_UTIL_H__

#include <glib.h>
#include <glib-object.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#define GCONF_TYPE_VALUE (gconf_value_get_type ())

GType gconf_value_get_type (void);
gchar *gconf_get_key_name_from_path (const gchar *path);
gchar *gconf_value_type_to_string (GConfValueType value_type);
GConfSchema *gconf_client_get_schema_for_key (GConfClient *client, const char *key);
gboolean gconf_util_can_edit_defaults (void);
gboolean gconf_util_can_edit_mandatory (void);

#endif /* __GCONF_UTIL_H__ */
