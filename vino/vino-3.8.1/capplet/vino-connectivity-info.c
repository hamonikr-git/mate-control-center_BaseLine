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

#include "config.h"

#include "vino-connectivity-info.h"

#include "vino-url-webservice.h"
#include "vino-dbus.h"

#include <libsoup/soup.h>
#include <string.h>

struct _VinoConnectivityInfo
{
  GObject      parent_instance;
  SoupSession *soup_session;
  GDBusProxy  *proxy;

  gboolean     stable;

  gchar       *internal_host;
  guint16      internal_port;
  gchar       *avahi_host;

  gchar       *external_host;
  guint16      external_port;

  /* either all set or all unset */
  guint16      checking_port;
  SoupMessage *checking_msg;
  guint        checking_timeout;
};

typedef GObjectClass VinoConnectivityInfoClass;

static guint vino_connectivity_info_changed_signal;

static GType vino_connectivity_info_get_type (void);
G_DEFINE_TYPE (VinoConnectivityInfo,
               vino_connectivity_info,
               G_TYPE_OBJECT);

#define VINO_WEBSERVICE_TIMEOUT 6

static void
vino_connectivity_info_changed (VinoConnectivityInfo *info,
                                gboolean              stable)
{
  if (info->stable || stable)
    {
      info->stable = stable;
      g_signal_emit (info, vino_connectivity_info_changed_signal, 0);
    }
}

static void
vino_connectivity_info_soup_finished (SoupSession *session,
                                      SoupMessage *msg,
                                      gpointer     user_data)
{
  VinoConnectivityInfo *info = user_data;
  GHashTable *table;
  gchar *ip = NULL;

  if (msg->status_code == SOUP_STATUS_OK &&
      soup_xmlrpc_extract_method_response (msg->response_body->data,
                                           msg->response_body->length,
                                           NULL,
                                           G_TYPE_HASH_TABLE, &table,
                                           G_TYPE_INVALID))
    {
      GHashTableIter iter;
      gboolean status;
      gpointer value;
      gpointer key;

      g_hash_table_iter_init (&iter, table);
      status = FALSE;

      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          if (strcmp (key, "status") == 0 && G_VALUE_HOLDS_BOOLEAN (value))
            status = g_value_get_boolean (value);

          else if (strcmp (key, "ip") == 0 && G_VALUE_HOLDS_STRING (value))
            ip = g_value_dup_string (value);
        }

      g_hash_table_unref (table);

      if (!status)
        {
          g_free (ip);
          ip = NULL;
        }
    }

  info->external_port = ip ? info->checking_port : 0;
  g_free (info->external_host);
  info->external_host = ip;

  g_source_remove (info->checking_timeout);
  info->checking_timeout = 0;
  info->checking_msg = NULL;
  info->checking_port = 0;

  vino_connectivity_info_changed (info, TRUE);
}

static gboolean
vino_connectivity_info_soup_timed_out (gpointer user_data)
{
  VinoConnectivityInfo *info = user_data;

  g_assert (info->checking_msg != NULL);
  soup_session_cancel_message (info->soup_session,
                               info->checking_msg,
                               SOUP_STATUS_REQUEST_TIMEOUT);
  g_assert (info->checking_msg == NULL);

  return FALSE;
}

static void
get_property_string (GDBusProxy   *proxy,
                     const gchar  *property,
                     gchar       **str)
{
  GVariant *value;

  g_free (*str);
  *str = NULL;

  if ((value = g_dbus_proxy_get_cached_property (proxy, property)))
    {
      if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING))
        {
          const gchar *tmp = g_variant_get_string (value, NULL);

          if (*tmp)
            *str = g_strdup (tmp);
        }
      g_variant_unref (value);
    }
}

static void
get_property_uint16 (GDBusProxy  *proxy,
                     const gchar *property,
                     guint16     *out)
{
  GVariant *value;

  *out = 0;

  if ((value = g_dbus_proxy_get_cached_property (proxy, property)))
    {
      if (g_variant_is_of_type (value, G_VARIANT_TYPE_UINT16))
        *out = g_variant_get_uint16 (value);
      g_variant_unref (value);
    }
}

static void
vino_connectivity_info_properties_changed (VinoConnectivityInfo *info)
{
  /* Cancel any in-flight webservice requests */
  if (info->checking_port)
    {
      soup_session_abort (info->soup_session);

      g_assert (info->checking_timeout == 0);
      g_assert (info->checking_msg == NULL);
      g_assert (info->checking_port == 0);
    }

  get_property_string (info->proxy, "ExternalHost", &info->external_host);
  get_property_uint16 (info->proxy, "ExternalPort", &info->external_port);
  get_property_string (info->proxy, "AvahiHost", &info->avahi_host);
  get_property_string (info->proxy, "Host", &info->internal_host);
  get_property_uint16 (info->proxy, "Port", &info->internal_port);

  /* No external host without external port */
  if (info->external_port == 0)
    {
      g_free (info->external_host);
      info->external_host = NULL;
    }

  /* No internal port without internal host */
  if (!info->internal_host)
    info->internal_port = 0;

  /* No internal or avahi hosts without internal port */
  if (info->internal_port == 0)
    {
      g_free (info->internal_host);
      info->internal_host = NULL;
      g_free (info->avahi_host);
      info->avahi_host = NULL;
    }

  if (!info->external_host)
    {
      /* We have no external hostname, so we can try and guess it using
       * the webservice to connect to the external port (if given) or
       * internal port (otherwise, if given).
       */
      if (info->external_port)
        info->checking_port = info->external_port;
      else
        info->checking_port = info->internal_port;

      info->external_port = 0;
    }

  if (info->checking_port)
    {
      gchar *url;

      url = vino_url_webservice_get_random ();

      if (url)
        {
          info->checking_msg =
            soup_xmlrpc_request_new (url, "vino.check",
                                     G_TYPE_INT, info->checking_port,
                                     G_TYPE_INT, VINO_WEBSERVICE_TIMEOUT,
                                     G_TYPE_INVALID);
          info->checking_timeout =
            g_timeout_add (1000 * (VINO_WEBSERVICE_TIMEOUT + 1),
                           vino_connectivity_info_soup_timed_out,
                           info);

          soup_session_queue_message (info->soup_session,
                                      info->checking_msg,
                                      vino_connectivity_info_soup_finished,
                                      info);
          g_free (url);
        }
      else
        info->checking_port = 0;
    }

  vino_connectivity_info_changed (info, info->checking_port == 0);
}

static void
vino_connectivity_info_proxy_ready (GObject      *source,
                                    GAsyncResult *result,
                                    gpointer      user_data)
{
  VinoConnectivityInfo *info = user_data;

  info->proxy = g_dbus_proxy_new_for_bus_finish (result, NULL);

  if (info->proxy)
    {
      g_signal_connect_swapped (info->proxy, "g-properties-changed",
        G_CALLBACK (vino_connectivity_info_properties_changed), info);

      vino_connectivity_info_properties_changed (info);
    }
  else
    vino_connectivity_info_changed (info, TRUE);
}

static void
vino_connectivity_info_finalize (GObject *object)
{
  VinoConnectivityInfo *info = (VinoConnectivityInfo *) object;

  if (info->proxy)
    g_object_unref (info->proxy);

  if (info->checking_timeout)
    g_source_remove (info->checking_timeout);

  soup_session_abort (info->soup_session);
  g_object_unref (info->soup_session);
  g_free (info->internal_host);
  g_free (info->external_host);
  g_free (info->avahi_host);

  G_OBJECT_CLASS (vino_connectivity_info_parent_class)
    ->finalize (object);
}

static void
vino_connectivity_info_init (VinoConnectivityInfo *info)
{
}

static void
vino_connectivity_info_class_init (GObjectClass *class)
{
  class->finalize = vino_connectivity_info_finalize;

  vino_connectivity_info_changed_signal =
    g_signal_new ("changed", vino_connectivity_info_get_type (),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

VinoConnectivityInfo *
vino_connectivity_info_new (gint screen_number)
{
  VinoConnectivityInfo *info;
  gchar *object_path;

  info = g_object_new (vino_connectivity_info_get_type (), NULL);
  info->soup_session = soup_session_async_new ();

  object_path = g_strdup_printf ("%s%d",
                                 ORG_GNOME_VINO_SCREEN_PATH_PREFIX,
                                 screen_number);
  g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                            ORG_GNOME_VINO_SCREEN_INTERFACE,
                            ORG_GNOME_VINO_BUS_NAME, object_path,
                            ORG_GNOME_VINO_SCREEN_INTERFACE_NAME, NULL,
                            vino_connectivity_info_proxy_ready, info);
  g_free (object_path);

  return info;

}

gboolean
vino_connectivity_info_get (VinoConnectivityInfo  *info,
                            gchar                **internal_host,
                            guint16               *internal_port,
                            gchar                **external_host,
                            guint16               *external_port,
                            gchar                **avahi_host)
{
  if (info->stable)
    {
      *external_host = g_strdup (info->external_host);
      *internal_host = g_strdup (info->internal_host);
      *avahi_host = g_strdup (info->avahi_host);
      *external_port = info->external_port;
      *internal_port = info->internal_port;

      return TRUE;
    }

  return FALSE;
}

void
vino_connectivity_info_check (VinoConnectivityInfo *info)
{
  vino_connectivity_info_properties_changed (info);
}
