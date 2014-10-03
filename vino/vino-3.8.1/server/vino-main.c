/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
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
 * Authors:
 *      Mark McLoughlin <mark@skynet.ie>
 *      Jonh Wendell <wendell@bani.com.br>
 *      Ryan Lortie <desrt@desrt.ca>
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "vino-input.h"
#include "vino-mdns.h"
#include "vino-server.h"
#include "vino-util.h"
#include "vino-dbus-listener.h"
#include "eggsmclient.h"

#ifdef VINO_HAVE_GNUTLS
#include <gnutls/gnutls.h>

#ifdef VINO_HAVE_TELEPATHY_GLIB
static gboolean tube = FALSE;
#endif

# ifdef GNOME_ENABLE_DEBUG
static void
vino_debug_gnutls (int         level,
                   const char *str)
{
  fputs (str, stderr);
}
# endif
#endif /* VINO_HAVE_GNUTLS */

typedef struct
{
  GSettings         *settings;
  GdkDisplay        *display;
  VinoDBusListener **listeners;
  gint               n_screens;
  EggSMClient       *sm_client;
  GMainLoop         *main_loop;
} VinoApplication;

typedef enum
{
  /* Screen sharing enabled, Vino should always be running */
  RUN_MODE_ALL,
  /* Screen sharing disabled but Vino started with --tube;
   * should exit once the tube has been handled */
  RUN_MODE_TUBE,
  /* Screen sharing disabled and Vino started without --tube;
   * should exit right away */
  RUN_MODE_EXIT,
} RunMode;

static RunMode
get_run_mode (VinoApplication *vino)
{
  if (g_settings_get_boolean (vino->settings, "enabled"))
    return RUN_MODE_ALL;

#ifdef VINO_HAVE_TELEPATHY_GLIB
  if (tube)
    return RUN_MODE_TUBE;
#endif

  return RUN_MODE_EXIT;
}

static void
set_all_servers_reject(VinoApplication *vino,
    gboolean reject)
{
  guint i;

  for (i = 0; i < vino->n_screens; i++)
    {
      VinoServer *server;

      server = vino_dbus_listener_get_server (vino->listeners[i]);

      vino_server_set_reject_incoming (server, reject);
    }
}

static void
enabled_changed (VinoApplication *vino)
{
  RunMode mode;
  gboolean reject;

  mode = get_run_mode (vino);
  if (mode == RUN_MODE_EXIT)
    {
      g_message ("The desktop sharing service has been disabled, exiting.");
      g_main_loop_quit (vino->main_loop);
      return;
    }

  if (mode == RUN_MODE_TUBE)
    {
      g_message ("The desktop sharing service has been disabled, "
          "reject network connections");
      reject = TRUE;
    }
  else
    {
      g_message ("The desktop sharing service has been enabled, "
          "accept network connections");
      reject = FALSE;
    }

  set_all_servers_reject (vino, reject);
}

#ifdef VINO_HAVE_TELEPATHY_GLIB
static void
tube_disconnected_cb (VinoDBusListener *listener,
    VinoTubeServer *server,
    VinoApplication *vino)
{
  if (get_run_mode (vino) != RUN_MODE_TUBE)
    return;

  g_message ("Tube has been closed, exiting.");
  g_main_loop_quit (vino->main_loop);
}
#endif

static void
bus_acquired (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  VinoApplication *vino = user_data;
  gint i;

  /* Get the listeners on their object paths before we acquire the name.
   *
   * This prevents incoming calls to the well-known name from finding
   * the listeners missing.
   *
   * Later (after we acquire the name) we will create the server objects
   * and register them with the listeners.
   */
  vino->display = gdk_display_get_default ();
  vino->n_screens = gdk_display_get_n_screens (vino->display);
  vino->listeners = g_new (VinoDBusListener *, vino->n_screens);
  for (i = 0; i < vino->n_screens; i++)
    {
      vino->listeners[i] = vino_dbus_listener_new (i);

#ifdef VINO_HAVE_TELEPATHY_GLIB
      g_signal_connect (vino->listeners[i], "tube-disconnected",
          G_CALLBACK (tube_disconnected_cb), vino);
#endif
    }
}

static void
name_acquired (GDBusConnection *connection,
               const gchar     *name,
               gpointer         user_data)
{
  VinoApplication *vino = user_data;
  gboolean view_only;
  gint i;
  gboolean reject = FALSE;

  if (get_run_mode (vino) == RUN_MODE_TUBE)
    {
      g_message ("Started in tube mode; reject network connections");
      reject = TRUE;
    }

  /* Name is acquired.  Start up the servers and register them with the
   * listeners.
   */
  if ((view_only = !vino_input_init (vino->display)))
    g_warning (_("Your XServer does not support the XTest extension - "
                 "remote desktop access will be view-only\n"));

  for (i = 0; i < vino->n_screens; i++)
    {
      VinoServer *server;

      /* The server is initially "on-hold" while we set everything up. */
      server = vino_server_new (gdk_display_get_screen (vino->display,
                                                        i),
                                view_only);

      g_settings_bind (vino->settings, "prompt-enabled",
                       server, "prompt-enabled", G_SETTINGS_BIND_GET);

      /* Only bind to the user's view-only preference if we're not
       * forced to be view-only by an incapable X server.
       */
      if (!view_only)
        g_settings_bind (vino->settings, "view-only",
                         server, "view-only", G_SETTINGS_BIND_GET);

      g_settings_bind (vino->settings, "network-interface",
                       server, "network-interface", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "use-alternative-port",
                       server, "use-alternative-port", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "alternative-port",
                       server, "alternative-port", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "authentication-methods",
                       server, "auth-methods", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "require-encryption",
                       server, "require-encryption", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "require-encryption",
                       server, "require-encryption", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "vnc-password",
                       server, "vnc-password", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "lock-screen-on-disconnect",
                       server, "lock-screen", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "disable-background",
                       server, "disable-background", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "use-upnp",
                       server, "use-upnp", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "disable-xdamage",
                       server, "disable-xdamage", G_SETTINGS_BIND_GET);
      g_settings_bind (vino->settings, "icon-visibility",
#ifdef VINO_ENABLE_APP_INDICATOR
                       vino_server_get_app_indicator (server),
#else
                       vino_server_get_status_icon (server),
#endif
                       "visibility", G_SETTINGS_BIND_GET);

      vino_dbus_listener_set_server (vino->listeners[i], server);
      vino_server_set_on_hold (server, FALSE);
      vino_server_set_reject_incoming (server, reject);

      if (g_settings_get_boolean (vino->settings, "enabled"))
        {
          vino_mdns_start(vino_server_get_network_interface (server));
        }

      g_object_unref (server);
    }
}

static void
name_lost (GDBusConnection *connection,
           const gchar     *name,
           gpointer         user_data)
{
  VinoApplication *vino = user_data;

  g_message ("The desktop sharing service is already running, exiting.");
  g_main_loop_quit (vino->main_loop);
}

static void
sm_quit (VinoApplication *vino)
{
  g_main_loop_quit (vino->main_loop);
}

int
main (int argc, char **argv)
{
  VinoApplication vino = { 0, };

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, VINO_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Parse commandline arguments */
  {
    GOptionContext *context;
    GError *error = NULL;
    GOptionEntry options[] = {
#ifdef VINO_HAVE_TELEPATHY_GLIB
      { "tube", 't',
        0, G_OPTION_ARG_NONE, &tube,
        /* Tube mode uses Telepathy's Tubes to share a user's desktop directly
         * with another IM contact. http://telepathy.freedesktop.org/wiki/Tubes
         */
        N_("Start in tube mode, for the ‘Share my Desktop’ feature"),
        NULL },
#endif
      { NULL }
    };

    context = g_option_context_new (_("- VNC Server for GNOME"));
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_add_group (context, egg_sm_client_get_option_group ());
    g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);

    if (!g_option_context_parse (context, &argc, &argv, &error))
      {
        g_print ("%s\n%s\n", error->message,
                 _("Run 'vino-server --help' to see a full list of "
                   "available command line options"));
        g_error_free (error);
        return 1;
      }
    g_option_context_free (context);
  }

  /* GSettings */
  vino.settings = g_settings_new ("org.gnome.Vino");
  g_signal_connect_swapped (vino.settings, "changed::enabled",
                            G_CALLBACK (enabled_changed), &vino);

  if (get_run_mode (&vino) == RUN_MODE_EXIT)
    {
      g_warning ("The desktop sharing service is not "
                 "enabled, so it should not be run.");
      return 1;
    }

  gtk_window_set_default_icon_name ("preferences-desktop-remote-desktop");
  g_set_application_name (_("GNOME Desktop Sharing"));
  vino.main_loop = g_main_loop_new (NULL, FALSE);

#ifdef GNOME_ENABLE_DEBUG
  vino_setup_debug_flags ();
# ifdef VINO_HAVE_GNUTLS
  if (_vino_debug_flags & VINO_DEBUG_TLS)
    {
      gnutls_global_set_log_level (10);
      gnutls_global_set_log_function (vino_debug_gnutls);
    }
# endif /* VINO_HAVE_GNUTLS */
#endif

  /* Session management */
  vino.sm_client = egg_sm_client_get ();
  egg_sm_client_set_mode (EGG_SM_CLIENT_MODE_NO_RESTART);
  g_signal_connect_swapped (vino.sm_client, "quit",
                            G_CALLBACK (sm_quit), &vino);


  /* Start attempting to acquire the bus name.
   * This starts everything...
   */
  g_bus_own_name (G_BUS_TYPE_SESSION, "org.gnome.Vino",
                  G_BUS_NAME_OWNER_FLAGS_NONE,
                  bus_acquired, name_acquired, name_lost,
                  &vino, NULL);


  g_main_loop_run (vino.main_loop);


  /* Shutdown */
  if (vino.listeners)
    {
      gint i;

      /* We need to finalize these to ensure
       * that mdns gets shutdown properly.
       */
      for (i = 0; i < vino.n_screens; i++)
        g_object_unref (vino.listeners[i]);

      g_free (vino.listeners);
    }

  vino_mdns_shutdown ();

  g_main_loop_unref (vino.main_loop);
  g_object_unref (vino.sm_client);
  g_object_unref (vino.settings);

  return 0;
}
