/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
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
 * Authors:
 *      Mark McLoughlin <mark@skynet.ie>
 */

#include "config.h"

#include "vino-prefs.h"


#include <string.h>
#include <glib/gstdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#include "vino-prefs.h"
#include "vino-util.h"
#include "vino-mdns.h"
#ifdef VINO_ENABLE_APP_INDICATOR
#include "vino-app-indicator.h"
#endif /* VINO_ENABLE_APP_INDICATOR */
#include "vino-status-icon.h"

static GSettings *settings       = NULL;
static gboolean   force_view_only;

static void
vino_prefs_restart_mdns (VinoServer *server,
                         gpointer    data)
{
  vino_mdns_stop ();
  vino_mdns_add_service ("_rfb._tcp", vino_server_get_port (server));
  vino_mdns_start (vino_server_get_network_interface (server));
}

VinoServer *
vino_prefs_create_server (GdkScreen *screen)
{
  VinoServer     *server;
#ifdef VINO_ENABLE_APP_INDICATOR
  VinoAppIndicator *indicator;
#else
  VinoStatusIcon *icon;
#endif /* VINO_ENABLE_APP_INDICATOR */

  /* Start the server 'on-hold' until all the settings are in, then go. */
  server = g_object_new (VINO_TYPE_SERVER,
                         "view-only", force_view_only,
                         "on-hold", TRUE,
                         "screen", screen,
                         NULL);
#ifdef VINO_ENABLE_APP_INDICATOR
  indicator = vino_server_get_app_indicator (server);
#else
  icon = vino_server_get_status_icon (server);
#endif /* VINO_ENABLE_APP_INDICATOR */

  g_settings_bind (settings, "prompt-enabled",
                   server, "prompt-enabled", G_SETTINGS_BIND_GET);

  if (!force_view_only)
    g_settings_bind (settings, "view-only",
                     server, "view-only", G_SETTINGS_BIND_GET);

  g_settings_bind (settings, "network-interface",
                   server, "network-interface", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "use-alternative-port",
                   server, "use-alternative-port", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "alternative-port",
                   server, "alternative-port", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "authentication-methods",
                   server, "auth-methods", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "require-encryption",
                   server, "require-encryption", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "require-encryption",
                   server, "require-encryption", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "vnc-password",
                   server, "vnc-password", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "lock-screen-on-disconnect",
                   server, "lock-screen", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "disable-background",
                   server, "disable-background", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "use-upnp",
                   server, "use-upnp", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "disable-xdamage",
                   server, "disable-xdamage", G_SETTINGS_BIND_GET);
  g_settings_bind (settings, "notify-on-connect",
                   server, "notify-on-connect", G_SETTINGS_BIND_GET);
#ifdef VINO_ENABLE_APP_INDICATOR
  g_settings_bind (settings, "icon-visibility",
                   indicator, "visibility", G_SETTINGS_BIND_GET);
#else
  g_settings_bind (settings, "icon-visibility",
                   icon, "visibility", G_SETTINGS_BIND_GET);
#endif /* VINO_ENABLE_APP_INDICATOR */

  vino_server_set_on_hold (server, FALSE);

  g_signal_connect (server, "notify::alternative-port", G_CALLBACK (vino_prefs_restart_mdns), NULL);
  g_signal_connect (server, "notify::use-alternative-port", G_CALLBACK(vino_prefs_restart_mdns), NULL);
  g_signal_connect (server, "notify::network-interface", G_CALLBACK (vino_prefs_restart_mdns), NULL);

  return server;
}

static void
vino_prefs_sighandler (int sig)
{
  g_message (_("Received signal %d, exiting."), sig);
  vino_mdns_shutdown ();
  vino_prefs_shutdown ();
  exit (0);
}

static void
notify_enabled (void)
{
  if (!g_settings_get_boolean (settings, "enabled"))
    {
      g_message ("The desktop sharing service is disabled, exiting.");
      exit (0);
    }
}

void
vino_prefs_init (gboolean view_only)
{
  settings = g_settings_new ("org.gnome.Vino");

  signal (SIGINT,  vino_prefs_sighandler); /* Ctrl+C */
  signal (SIGQUIT, vino_prefs_sighandler);
  signal (SIGTERM, vino_prefs_sighandler); /* kill -15 */
  signal (SIGSEGV, vino_prefs_sighandler); /* Segmentation fault */

  g_signal_connect (settings, "changed::enabled",
                    G_CALLBACK (notify_enabled), NULL);
  notify_enabled ();

  force_view_only = view_only;
}

void
vino_prefs_shutdown (void)
{
  g_object_unref (settings);
  settings = NULL;
}
