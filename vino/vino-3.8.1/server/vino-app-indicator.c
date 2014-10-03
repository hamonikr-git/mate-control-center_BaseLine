/*
 * Copyright (C) 2010 Canonical Ltd
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
 *      Travis B. Hartwell <nafai@travishartwell.net>
 */

#include <config.h>
#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>
#include <string.h>
#ifdef VINO_ENABLE_LIBNOTIFY
#include <libnotify/notify.h>
#endif
#include <libappindicator/app-indicator.h>

#include "vino-app-indicator.h"
#include "vino-enums.h"
#include "vino-util.h"

struct _VinoAppIndicatorPrivate
{
  AppIndicator *indicator;
  GtkMenu    *menu;
  GtkMenuItem *status_menu_item;
  VinoServer *server;
  GSList     *clients;
  GtkWidget  *disconnect_dialog;
  VinoStatusIconVisibility visibility;

#ifdef VINO_ENABLE_LIBNOTIFY
  NotifyNotification *new_client_notification;
#endif
};

G_DEFINE_TYPE (VinoAppIndicator, vino_app_indicator, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_SERVER,
  PROP_VISIBILITY
};

typedef struct
{
  VinoAppIndicator *indicator;
  VinoClient     *client;
} VinoAppIndicatorNotify;

static gboolean vino_app_indicator_show_new_client_notification (gpointer user_data);
static void vino_app_indicator_build_menu (VinoAppIndicator *indicator);
static void vino_app_indicator_setup_menu (VinoAppIndicator *indicator);

static void
vino_app_indicator_finalize (GObject *object)
{
  VinoAppIndicator *indicator = VINO_APP_INDICATOR (object);

#ifdef VINO_ENABLE_LIBNOTIFY
  if (indicator->priv->new_client_notification)
    g_object_unref (indicator->priv->new_client_notification);
  indicator->priv->new_client_notification = NULL;
#endif

  if (indicator->priv->status_menu_item)
    gtk_widget_destroy (GTK_WIDGET (indicator->priv->status_menu_item));
  indicator->priv->status_menu_item = NULL;

  if (indicator->priv->menu)
    gtk_widget_destroy (GTK_WIDGET (indicator->priv->menu));
  indicator->priv->menu = NULL;

  if (indicator->priv->clients)
    g_slist_free (indicator->priv->clients);
  indicator->priv->clients = NULL;

  if (indicator->priv->disconnect_dialog)
    gtk_widget_destroy (indicator->priv->disconnect_dialog);
  indicator->priv->disconnect_dialog = NULL;

  if (indicator->priv->indicator)
    g_object_unref (indicator->priv->indicator);
  indicator->priv->indicator = NULL;

  G_OBJECT_CLASS (vino_app_indicator_parent_class)->finalize (object);
}

void
vino_app_indicator_update_state (VinoAppIndicator *indicator)
{
  char     *tooltip;
  gboolean visible;
  AppIndicatorStatus status;

  g_return_if_fail (VINO_IS_APP_INDICATOR (indicator));

  vino_app_indicator_setup_menu (indicator);

  visible = !vino_server_get_on_hold (indicator->priv->server);

  tooltip = g_strdup (_("Desktop sharing is enabled"));

  if (indicator->priv->clients != NULL)
    {
      int n_clients;

      n_clients = g_slist_length (indicator->priv->clients);

      tooltip = g_strdup_printf (ngettext ("One person is connected",
                                           "%d people are connected",
                                           n_clients),
                                 n_clients);
      visible = (visible) && ( (indicator->priv->visibility == VINO_STATUS_ICON_VISIBILITY_CLIENT) ||
			     (indicator->priv->visibility == VINO_STATUS_ICON_VISIBILITY_ALWAYS) );
    }
  else
    visible = visible && (indicator->priv->visibility == VINO_STATUS_ICON_VISIBILITY_ALWAYS);

  gtk_menu_item_set_label (GTK_MENU_ITEM (indicator->priv->status_menu_item), tooltip);

  status = visible ? APP_INDICATOR_STATUS_ACTIVE : APP_INDICATOR_STATUS_PASSIVE;
  if (indicator->priv->indicator != NULL)
	  app_indicator_set_status (APP_INDICATOR (indicator->priv->indicator), status);

  g_free (tooltip);
}

static void
vino_app_indicator_init (VinoAppIndicator *indicator)
{
  indicator->priv = G_TYPE_INSTANCE_GET_PRIVATE (indicator, VINO_TYPE_APP_INDICATOR, VinoAppIndicatorPrivate);

  indicator->priv->indicator = app_indicator_new ("vino-app-indicator",
						  "preferences-desktop-remote-desktop",
						  APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  indicator->priv->menu = NULL;
  indicator->priv->status_menu_item = NULL;

  vino_app_indicator_setup_menu (indicator);
}

static void
vino_app_indicator_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
  VinoAppIndicator *indicator = VINO_APP_INDICATOR (object);

  switch (prop_id)
    {
    case PROP_SERVER:
      indicator->priv->server = g_value_get_object (value);
      break;
    case PROP_VISIBILITY:
      vino_app_indicator_set_visibility (indicator, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vino_app_indicator_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
  VinoAppIndicator *indicator = VINO_APP_INDICATOR (object);

  switch (prop_id)
    {
    case PROP_SERVER:
      g_value_set_object (value, indicator->priv->server);
      break;
    case PROP_VISIBILITY:
      g_value_set_enum (value, indicator->priv->visibility);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

VinoAppIndicator *
vino_app_indicator_new (VinoServer *server)
{
  g_return_val_if_fail (VINO_IS_SERVER (server), NULL);

  return g_object_new (VINO_TYPE_APP_INDICATOR,
                       "server",    server,
                       NULL);
}

VinoServer *
vino_app_indicator_get_server (VinoAppIndicator *indicator)
{
  g_return_val_if_fail (VINO_IS_APP_INDICATOR (indicator), NULL);

  return indicator->priv->server;
}

static void
vino_app_indicator_preferences (VinoAppIndicator *indicator)
{
  GdkScreen *screen;
  GDesktopAppInfo *info;
  GdkAppLaunchContext *context;
  GError *error = NULL;

  screen = gdk_screen_get_default();
  info = g_desktop_app_info_new ("vino-preferences.desktop");
  context = gdk_display_get_app_launch_context (gdk_screen_get_display (screen));
  if (!g_app_info_launch (G_APP_INFO (info), NULL, G_APP_LAUNCH_CONTEXT (context), &error))
    {
      vino_util_show_error (_("Error displaying preferences"),
                            error->message,
                            NULL);
      g_error_free (error);
    }

  g_object_unref (info);
  g_object_unref (context);
}

static void
vino_app_indicator_help (VinoAppIndicator *indicator)
{
  GdkScreen *screen;
  GError    *error = NULL;

  screen = gdk_screen_get_default ();
  if (!gtk_show_uri (screen,
		     "ghelp:user-guide?goscustdesk-90",
		     GDK_CURRENT_TIME,
		     &error))
    {
      vino_util_show_error (_("Error displaying help"),
			    error->message,
			    NULL);
      g_error_free (error);
    }
}

static void
vino_app_indicator_about (VinoAppIndicator *indicator)
{

  g_return_if_fail (VINO_IS_APP_INDICATOR (indicator));

  const char *authors[] = {
    "Mark McLoughlin <mark@skynet.ie>",
    "Calum Benson <calum.benson@sun.com>",
    "Federico Mena Quintero <federico@ximian.com>",
    "Sebastien Estienne <sebastien.estienne@gmail.com>",
    "Shaya Potter <spotter@cs.columbia.edu>",
    "Steven Zhang <steven.zhang@sun.com>",
    "Srirama Sharma <srirama.sharma@wipro.com>",
    "Jonh Wendell <wendell@bani.com.br>",
    NULL
  };
  char *license;
  char *translators;

  license = _("Licensed under the GNU General Public License Version 2\n\n"
              "Vino is free software; you can redistribute it and/or\n"
              "modify it under the terms of the GNU General Public License\n"
              "as published by the Free Software Foundation; either version 2\n"
              "of the License, or (at your option) any later version.\n\n"
              "Vino is distributed in the hope that it will be useful,\n"
              "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
              "GNU General Public License for more details.\n\n"
              "You should have received a copy of the GNU General Public License\n"
              "along with this program; if not, write to the Free Software\n"
              "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n"
              "02110-1301, USA.\n");

  /* Translators comment: put your own name here to appear in the about dialog. */
  translators = _("translator-credits");

  if (!strcmp (translators, "translator-credits"))
    translators = NULL;

  gtk_show_about_dialog (NULL,
                         "comments",           _("Share your desktop with other users"),
                         "version",            PACKAGE_VERSION,
                         "license",            license,
                         "authors",            authors,
                         "translator-credits", translators,
                         "logo-icon-name",     "preferences-desktop-remote-desktop",
                         NULL);
}

static void
vino_app_indicator_disconnect_client (VinoAppIndicatorNotify *a, gint response)
{

  GSList *l;
  GSList *next;

  VinoAppIndicator *indicator    = a->indicator;
  VinoClient     *client  = a->client;

  gtk_widget_destroy (indicator->priv->disconnect_dialog);
  indicator->priv->disconnect_dialog = NULL;

  if (response != GTK_RESPONSE_OK)
  {
    g_free (a);
    return;
  }

  if (client)
  {
    if (g_slist_find (indicator->priv->clients, client))
      vino_client_disconnect (client);
  }
  else
    for (l = indicator->priv->clients; l; l = next)
      {
        VinoClient *client = l->data;

        next = l->next;

        vino_client_disconnect (client);
      }

  g_free (a);
}

static void
vino_app_indicator_disconnect_confirm (VinoAppIndicatorNotify *a)
{
  char      *primary_msg;
  char      *secondary_msg;

  VinoAppIndicator *indicator    = a->indicator;
  VinoClient     *client  = a->client;

  if (indicator->priv->disconnect_dialog)
  {
    gtk_window_present (GTK_WINDOW (indicator->priv->disconnect_dialog));
    return;
  }

  if (client != NULL)
    {
      /* Translators: %s is a hostname */
      primary_msg   = g_strdup_printf
          (_("Are you sure you want to disconnect '%s'?"),
          vino_client_get_hostname (client));
      secondary_msg = g_strdup_printf
          (_("The remote user from '%s' will be disconnected. Are you sure?"),
          vino_client_get_hostname (client));
    }
  else
    {
      primary_msg   = g_strdup
          (_("Are you sure you want to disconnect all clients?"));
      secondary_msg = g_strdup
          (_("All remote users will be disconnected. Are you sure?"));
    }

  indicator->priv->disconnect_dialog = gtk_message_dialog_new (NULL,
                                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                                          GTK_MESSAGE_QUESTION,
                                                          GTK_BUTTONS_CANCEL,
                                                          "%s",
                                                          primary_msg);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (indicator->priv->disconnect_dialog), FALSE);

  gtk_dialog_add_button (GTK_DIALOG (indicator->priv->disconnect_dialog), _("Disconnect"), GTK_RESPONSE_OK);

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (indicator->priv->disconnect_dialog),
                                            "%s", secondary_msg);

  g_signal_connect_swapped (indicator->priv->disconnect_dialog, "response",
                            G_CALLBACK (vino_app_indicator_disconnect_client), (gpointer) a);
  gtk_widget_show_all (GTK_WIDGET (indicator->priv->disconnect_dialog));

  g_free (primary_msg);
  g_free (secondary_msg);
}

static void
vino_app_indicator_setup_menu (VinoAppIndicator *indicator)
{
  if (indicator->priv->status_menu_item)
    gtk_widget_destroy (GTK_WIDGET (indicator->priv->status_menu_item));
  indicator->priv->status_menu_item = NULL;

  if (indicator->priv->menu)
    gtk_widget_destroy (GTK_WIDGET (indicator->priv->menu));
  indicator->priv->menu = NULL;

  vino_app_indicator_build_menu (indicator);

  app_indicator_set_menu (indicator->priv->indicator, indicator->priv->menu);
}

static void
vino_app_indicator_build_menu (VinoAppIndicator *indicator)
{
  GtkWidget      *item;
  GSList         *l;
  VinoAppIndicatorNotify *a;
  guint          n_clients;

  indicator->priv->menu = (GtkMenu*) gtk_menu_new ();

  indicator->priv->status_menu_item = (GtkMenuItem*) gtk_menu_item_new ();
  gtk_widget_set_sensitive (GTK_WIDGET (indicator->priv->status_menu_item), FALSE);
  gtk_widget_show (GTK_WIDGET (indicator->priv->status_menu_item));
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), GTK_WIDGET (indicator->priv->status_menu_item));

  item = gtk_separator_menu_item_new ();
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  item = gtk_image_menu_item_new_with_mnemonic (_("_Preferences"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                 gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (vino_app_indicator_preferences), indicator);
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  item = gtk_separator_menu_item_new ();
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  n_clients = g_slist_length (indicator->priv->clients);
  if (n_clients > 1)
    {
      item  = gtk_image_menu_item_new_with_label (_("Disconnect all"));
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                     gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU));

      a = g_new (VinoAppIndicatorNotify, 1);
      a->indicator = indicator;
      a->client = NULL;

      g_signal_connect_swapped (item, "activate",
                                G_CALLBACK (vino_app_indicator_disconnect_confirm), (gpointer) a);
      gtk_widget_show (item);
      gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);
    }

  for (l = indicator->priv->clients; l; l = l->next)
    {
      VinoClient *client = l->data;
      char       *str;

      a = g_new (VinoAppIndicatorNotify, 1);
      a->indicator   = indicator;
      a->client = client;

      /* Translators: %s is a hostname */
      str = g_strdup_printf (_("Disconnect %s"),
          vino_client_get_hostname (client));

      item  = gtk_image_menu_item_new_with_label (str);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                     gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU));
      g_signal_connect_swapped (item, "activate",
                                G_CALLBACK (vino_app_indicator_disconnect_confirm), (gpointer) a);
      gtk_widget_show (item);
      gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

      g_free (str);
    }

  if (n_clients)
    {
      item = gtk_separator_menu_item_new ();
      gtk_widget_show (item);
      gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);
    }

  item = gtk_image_menu_item_new_with_mnemonic (_("_Help"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
				 gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU));
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (vino_app_indicator_help), indicator);
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  item = gtk_image_menu_item_new_with_mnemonic (_("_About"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                 gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU));
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (vino_app_indicator_about), indicator);
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);
}

void
vino_app_indicator_add_client (VinoAppIndicator *indicator,
			       VinoClient     *client)
{
  g_return_if_fail (VINO_IS_APP_INDICATOR (indicator));
  g_return_if_fail (client != NULL);

  indicator->priv->clients = g_slist_append (indicator->priv->clients, client);

  vino_app_indicator_update_state (indicator);

  /* Visible if not APP_INDICATOR_STATUS_PASSIVE */
  if (app_indicator_get_status (indicator->priv->indicator) != APP_INDICATOR_STATUS_PASSIVE)
    {
      VinoAppIndicatorNotify *a;

      a = g_new (VinoAppIndicatorNotify, 1);
      a->indicator   = indicator;
      a->client = client;
      g_timeout_add_seconds (1,
                             vino_app_indicator_show_new_client_notification,
                             (gpointer) a);
    }
}

gboolean
vino_app_indicator_remove_client (VinoAppIndicator *indicator,
				  VinoClient     *client)
{
  g_return_val_if_fail (VINO_IS_APP_INDICATOR (indicator), TRUE);
  g_return_val_if_fail (client != NULL, TRUE);

  if (!indicator->priv->clients)
    return FALSE;

  indicator->priv->clients = g_slist_remove (indicator->priv->clients, client);

  vino_app_indicator_update_state (indicator);

  return indicator->priv->clients == NULL;
}

static void
vino_app_indicator_class_init (VinoAppIndicatorClass *klass)
{
  GObjectClass       *gobject_class     = G_OBJECT_CLASS (klass);

  gobject_class->finalize     = vino_app_indicator_finalize;
  gobject_class->set_property = vino_app_indicator_set_property;
  gobject_class->get_property = vino_app_indicator_get_property;

  g_object_class_install_property (gobject_class,
				   PROP_SERVER,
				   g_param_spec_object ("server",
							"Server",
							"The server",
							VINO_TYPE_SERVER,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_NAME |
							G_PARAM_STATIC_NICK |
							G_PARAM_STATIC_BLURB));
  g_object_class_install_property (gobject_class,
				   PROP_VISIBILITY,
				   g_param_spec_enum ("visibility",
						      "Indicator visibility",
						      "When the indicator must be shown",
						      VINO_TYPE_STATUS_ICON_VISIBILITY,
						      VINO_STATUS_ICON_VISIBILITY_ALWAYS,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT |
						      G_PARAM_STATIC_NAME |
						      G_PARAM_STATIC_NICK |
						      G_PARAM_STATIC_BLURB));

  g_type_class_add_private (gobject_class, sizeof (VinoAppIndicatorPrivate));
}

#ifdef VINO_ENABLE_LIBNOTIFY
static void
vino_status_handle_new_client_notification_closed (VinoAppIndicator *indicator)
{
  g_object_unref (indicator->priv->new_client_notification);
  indicator->priv->new_client_notification = NULL;
}
#endif /* VINO_ENABLE_LIBNOTIFY */

static gboolean
vino_app_indicator_show_new_client_notification (gpointer user_data)
{
#ifdef VINO_ENABLE_LIBNOTIFY
#define NOTIFICATION_TIMEOUT 5

  GError     *error;
  const char *summary;
  char       *body;

  VinoAppIndicatorNotify *a = (VinoAppIndicatorNotify *)user_data;
  VinoAppIndicator *indicator    = a->indicator;
  VinoClient     *client  = a->client;

  if (vino_server_get_prompt_enabled (indicator->priv->server))
  {
    g_free (user_data);
    return FALSE;
  }

  if (!notify_is_initted () &&  !notify_init (g_get_application_name ()))
    {
      g_printerr (_("Error initializing libnotify\n"));
      g_free (user_data);
      return FALSE;
    }

  if (g_slist_index (indicator->priv->clients, client) == -1)
    {
      g_free (user_data);
      return FALSE;
    }

  if (indicator->priv->new_client_notification)
    {
      notify_notification_close (indicator->priv->new_client_notification, NULL);
      g_object_unref (indicator->priv->new_client_notification);
      indicator->priv->new_client_notification = NULL;
    }

  if (vino_server_get_view_only (indicator->priv->server))
    {
      /* Translators: %s is a hostname */
      summary = _("Another user is viewing your desktop");
      body = g_strdup_printf
          (_("A user on the computer '%s' is remotely viewing your desktop."),
          vino_client_get_hostname (client));
    }
  else
    {
      /* Translators: %s is a hostname */
      summary = _("Another user is controlling your desktop");
      body = g_strdup_printf
          (_("A user on the computer '%s' is remotely controlling "
          "your desktop."), vino_client_get_hostname (client));
    }

  indicator->priv->new_client_notification =
    notify_notification_new (summary,
			     body,
			     "preferences-desktop-remote-desktop"
			     );

  g_free (body);

  g_signal_connect_swapped (indicator->priv->new_client_notification, "closed",
                            G_CALLBACK (vino_status_handle_new_client_notification_closed),
                            indicator);

  notify_notification_set_timeout (indicator->priv->new_client_notification,
                                   NOTIFICATION_TIMEOUT * 1000);

  error = NULL;
  if (!notify_notification_show (indicator->priv->new_client_notification, &error))
    {
      g_printerr (_("Error while displaying notification bubble: %s\n"),
                  error->message);
      g_error_free (error);
    }

  g_free (user_data);

#undef NOTIFICATION_TIMEOUT
#endif /* VINO_ENABLE_LIBNOTIFY */

  return FALSE;
}

void
vino_app_indicator_set_visibility (VinoAppIndicator *indicator,
				   VinoStatusIconVisibility  visibility)
{
  g_return_if_fail (VINO_IS_APP_INDICATOR (indicator));
  g_return_if_fail (visibility != VINO_STATUS_ICON_VISIBILITY_INVALID);

  if (visibility != indicator->priv->visibility)
    {
      indicator->priv->visibility = visibility;
      vino_app_indicator_update_state (indicator);
    }
}

guint
vino_app_indicator_get_visibility (VinoAppIndicator *indicator)
{
  g_return_val_if_fail (VINO_IS_APP_INDICATOR (indicator), VINO_STATUS_ICON_VISIBILITY_INVALID);

  return indicator->priv->visibility;
}
