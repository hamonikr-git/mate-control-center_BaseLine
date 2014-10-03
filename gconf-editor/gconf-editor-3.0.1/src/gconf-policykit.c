/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Vincent Untz <vuntz@gnome.org>
 *
 * Based on set-timezone.c from gnome-panel which was GPLv2+ with this
 * copyright:
 *    Copyright (C) 2007 David Zeuthen <david@fubar.dk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <gio/gio.h>

#include "gconf-policykit.h"

#define CACHE_VALIDITY_SEC 10

static guint
can_set (const gchar *key, gboolean mandatory)
{
        GDBusConnection *bus = NULL;
        GDBusProxy *proxy = NULL;
	const gchar *keys[2];
	const gchar *func;
	GError *error = NULL;
        GVariant *res;
        guint retval = 0;

        bus = g_bus_get_sync (G_BUS_TYPE_SYSTEM,
                              NULL, NULL);
        if (bus == NULL)
                goto out;

        proxy = g_dbus_proxy_new_sync (bus,
                                       G_DBUS_PROXY_FLAGS_NONE,
                                       NULL,
                                       "org.gnome.GConf.Defaults",
                                       "/",
                                       "org.gnome.GConf.Defaults",
                                       NULL, NULL);
        if (proxy == NULL)
                goto out;

	keys[0] = key;
	keys[1] = NULL;
	func = mandatory ? "CanSetMandatory" : "CanSetSystem";

        res = g_dbus_proxy_call_sync (proxy, func,
                                      g_variant_new ("(^as)", keys),
                                      G_DBUS_CALL_FLAGS_NONE,
                                      -1, NULL, &error);

        if (error != NULL) {
    		g_warning ("error calling %s: %s\n", func, error->message);
    		g_error_free (error);
  	} else {
                g_variant_get (res, "(u)", &retval);
        }

out:
        if (bus)
                g_object_unref (bus);

	if (proxy)
		g_object_unref (proxy);

        return retval;
}

typedef struct
{
	time_t last_refreshed;
	gint can_set;
} CacheEntry;

static GHashTable *defaults_cache = NULL;
static GHashTable *mandatory_cache = NULL;

gint
gconf_pk_can_set (const gchar *key, gboolean mandatory)
{
        time_t now;
	GHashTable **cache;
	CacheEntry *entry;

        time (&now);
	cache = mandatory ? &mandatory_cache : &defaults_cache;
	if (!*cache)
		*cache = g_hash_table_new (g_str_hash, g_str_equal);
	entry = (CacheEntry *) g_hash_table_lookup (*cache, key);
	if (!entry) {
		entry = g_new0 (CacheEntry, 1);
		g_hash_table_insert (*cache, (char *) key, entry);
	}
        if (ABS (now - entry->last_refreshed) > CACHE_VALIDITY_SEC) {
        	entry->can_set = can_set (key, mandatory);
                entry->last_refreshed = now;
        }

        return entry->can_set;
}

gint
gconf_pk_can_set_default (const gchar *key)
{
	return gconf_pk_can_set (key, FALSE);
}

gint
gconf_pk_can_set_mandatory (const gchar *key)
{
	return gconf_pk_can_set (key, TRUE);
}

static void
gconf_pk_update_can_set_cache (const gchar *key,
                               gboolean     mandatory)
{
        time_t          now;
	GHashTable **cache;
	CacheEntry *entry;

        time (&now);
	cache = mandatory ? &mandatory_cache : &defaults_cache;
	if (!*cache)
		*cache = g_hash_table_new (g_str_hash, g_str_equal);
	entry = (CacheEntry *) g_hash_table_lookup (*cache, key);
	if (!entry) {
		entry = g_new0 (CacheEntry, 1);
		g_hash_table_insert (*cache, (char *) key, entry);
	}
        entry->can_set = 2;
        entry->last_refreshed = now;
}

typedef struct {
        gint            ref_count;
        gboolean        mandatory;
        gchar          *key;
        GFunc           callback;
        gpointer        data;
        GDestroyNotify  notify;
} GConfPKCallbackData;

static void set_key_async (GConfPKCallbackData *data);

static void
gconf_pk_update_can_set_cache (const gchar *key,
                               gboolean     mandatory);

static void
on_proxy_method_call (GObject *source,
                      GAsyncResult *res,
                      gpointer user_data)
{
        GConfPKCallbackData *data = user_data;
        GError *error = NULL;

        g_dbus_proxy_call_finish (G_DBUS_PROXY (source), res, &error);

        if (error == NULL) {
		gconf_pk_update_can_set_cache (data->key, data->mandatory);
                if (data->callback)
                        data->callback (data->data, NULL);
        } else {
                if (error->domain == G_DBUS_ERROR &&
                    error->code == G_DBUS_ERROR_NO_REPLY) {
                        /* these errors happen because dbus doesn't
                         * use monotonic clocks
                         */
                        g_warning ("ignoring no-reply error when setting key");
                        g_error_free (error);
                        if (data->callback)
                                data->callback (data->data, NULL);
                }
                else {
                        if (data->callback)
                                data->callback (data->data, error);
                }
        }
}

static void
on_gconf_proxy_created (GObject *source,
                        GAsyncResult *res,
                        gpointer user_data)
{
        GConfPKCallbackData *data = user_data;
        GDBusProxy *proxy;
	const gchar     *call;
        gchar *keys[2] = { data->key, NULL };
        gchar *dummy[2] = { NULL };
        GError *error = NULL;

        proxy = g_dbus_proxy_new_finish (res, &error);

        if (error != NULL) {
                if (data->callback)
                        data->callback (data->data, error);

                return;
        }

	call = data->mandatory ? "SetMandatory" : "SetSystem";
        g_dbus_proxy_call (proxy,
                           call, 
                           g_variant_new ("(^as^as)",
                                          keys, dummy),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1, NULL,
                           on_proxy_method_call, data);
}

static void
on_system_bus_got (GObject *source,
                   GAsyncResult *res,
                   gpointer user_data)
{
        GConfPKCallbackData *data = user_data;
        GDBusConnection *bus;
        GError *error = NULL;

        bus = g_bus_get_finish (res, &error);

        if (error != NULL) {
                if (data->callback)
                        data->callback (data->data, error);

                return;
        }

        g_dbus_proxy_new (bus,
                          G_DBUS_PROXY_FLAGS_NONE,
                          NULL,
                          "org.gnome.GConf.Defaults",
                          "/",
                          "org.gnome.GConf.Defaults",
                          NULL,
                          on_gconf_proxy_created,
                          data);
}

static void
set_key_async (GConfPKCallbackData *data)
{
        g_bus_get (G_BUS_TYPE_SYSTEM,
                   NULL, on_system_bus_got, data);
}

void
gconf_pk_set_default_async (const gchar    *key,
                            GFunc           callback,
                            gpointer        d,
                            GDestroyNotify  notify)
{
        GConfPKCallbackData *data;

        if (key == NULL)
                return;

        data = g_slice_new0 (GConfPKCallbackData);
        data->mandatory = FALSE;
        data->key = g_strdup (key);
        data->callback = callback;
        data->data = d;
        data->notify = notify;

        set_key_async (data);
}

void
gconf_pk_set_mandatory_async (const gchar    *key,
                              GFunc           callback,
                              gpointer        d,
                              GDestroyNotify  notify)
{
        GConfPKCallbackData *data;

        if (key == NULL)
                return;

        data = g_slice_new0 (GConfPKCallbackData);
        data->mandatory = TRUE;
        data->key = g_strdup (key);
        data->callback = callback;
        data->data = d;
        data->notify = notify;

        set_key_async (data);
}
