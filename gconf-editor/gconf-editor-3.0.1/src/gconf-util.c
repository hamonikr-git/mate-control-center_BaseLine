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

#include <config.h>
#include "gconf-util.h"

#include <string.h>

enum {
	GCONF_NOT_SET,
	GCONF_CANNOT_EDIT,
	GCONF_CAN_EDIT
};
 
GType
gconf_value_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static ("GConfValue",
						     (GBoxedCopyFunc)gconf_value_copy,
						     (GBoxedFreeFunc)gconf_value_free);
	}

	return type;
}

gchar *
gconf_get_key_name_from_path (const gchar *path)
{
	const gchar *ptr;

	/* FIXME:  VALIDATE KEY */
	
	ptr = path + strlen (path);

	while (ptr[-1] != '/')
		ptr--;

	return g_strdup (ptr);
}

gchar *
gconf_value_type_to_string (GConfValueType value_type)
{
	switch (value_type) {
	case GCONF_VALUE_STRING:
		return g_strdup ("String");
		break;
	case GCONF_VALUE_INT:
		return g_strdup ("Integer");
		break;
	case GCONF_VALUE_BOOL:
		return g_strdup ("Boolean");
		break;
	case GCONF_VALUE_LIST:
		return g_strdup ("List");
		break;
	default:
		return g_strdup_printf ("UNKNOWN, %d", value_type);
	}
}

GConfSchema *
gconf_client_get_schema_for_key (GConfClient *client, const char *key)
{
	GConfEntry *entry;
	const char *schema_name;
	GConfSchema *schema = NULL;

	entry = gconf_client_get_entry (client, key, NULL, TRUE, NULL);
	schema_name = gconf_entry_get_schema_name (entry);

	if (schema_name == NULL)
		goto out;

	schema = gconf_client_get_schema (client, schema_name, NULL);

out:
	if (entry)
		gconf_entry_free (entry);

	return schema;
}

static gboolean
can_edit_source (const char *source)
{
	GConfEngine *engine;
	GConfClient *client;
	GConfEntry  *entry;
	GError      *error;
	gboolean     retval;

	if (!(engine = gconf_engine_get_for_address (source, NULL)))
		return FALSE;

	error = NULL;
	client = gconf_client_get_for_engine (engine);
	entry = gconf_client_get_entry (client,
					"/apps/gconf-editor/can_edit_source",
					NULL,
					FALSE,
					&error);
	if (error != NULL) {
		g_assert (entry == NULL);
		g_error_free (error);
		g_object_unref (client);
		gconf_engine_unref (engine);
		return FALSE;
	}

	g_assert (entry != NULL);

	retval = gconf_entry_get_is_writable (entry);

	gconf_entry_unref (entry);
	g_object_unref (client);
	gconf_engine_unref (engine);

	return retval;
}

gboolean
gconf_util_can_edit_defaults (void)
{
	static int can_edit_defaults = GCONF_NOT_SET;

	if (can_edit_defaults == GCONF_NOT_SET)
		can_edit_defaults = can_edit_source (GCONF_DEFAULTS_SOURCE);

	return can_edit_defaults;
}

gboolean
gconf_util_can_edit_mandatory (void)
{
	static int can_edit_mandatory = GCONF_NOT_SET;

	if (can_edit_mandatory == GCONF_NOT_SET)
		can_edit_mandatory = can_edit_source (GCONF_MANDATORY_SOURCE);

	return can_edit_mandatory;
}

	

