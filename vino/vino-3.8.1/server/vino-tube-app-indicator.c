/*
 * © 2010, Canonical Ltd
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
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>
#ifdef VINO_ENABLE_LIBNOTIFY
#include <glib/gi18n.h>
#include <libnotify/notify.h>
#endif
#include <libappindicator/app-indicator.h>

#include "vino-tube-app-indicator.h"
#include "vino-enums.h"
#include "vino-util.h"

static void vino_tube_app_indicator_disconnect_confirm (VinoTubeAppIndicator *indicator);
static void vino_tube_app_indicator_help (VinoTubeAppIndicator *indicator);
static void vino_tube_app_indicator_preferences (VinoTubeAppIndicator *indicator);

struct _VinoTubeAppIndicatorPrivate
{
  AppIndicator *indicator;
  GtkMenu *menu;
  GtkMenuItem *status_menu_item;
  VinoTubeServer *server;
  GtkWidget *disconnect_dialog;
  VinoStatusTubeIconVisibility visibility;

#ifdef VINO_ENABLE_LIBNOTIFY
  NotifyNotification *new_client_notification;
#endif
};

G_DEFINE_TYPE (VinoTubeAppIndicator, vino_tube_app_indicator, G_TYPE_OBJECT);

enum
{
  PROP_0,
  PROP_SERVER,
  PROP_VISIBILITY
};

static void
vino_tube_app_indicator_finalize (GObject *object)
{
  VinoTubeAppIndicator *indicator = VINO_TUBE_APP_INDICATOR (object);

#ifdef VINO_ENABLE_LIBNOTIFY
  if (indicator->priv->new_client_notification != NULL)
    {
      notify_notification_close (indicator->priv->new_client_notification, NULL);
      g_object_unref (indicator->priv->new_client_notification);
      indicator->priv->new_client_notification = NULL;
    }
 #endif

  if (indicator->priv->status_menu_item != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (indicator->priv->status_menu_item));
      indicator->priv->status_menu_item = NULL;
    }
  if (indicator->priv->menu != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (indicator->priv->menu));
      indicator->priv->menu = NULL;
    }

  if (indicator->priv->disconnect_dialog != NULL)
    {
      gtk_widget_destroy (indicator->priv->disconnect_dialog);
      indicator->priv->disconnect_dialog = NULL;
    }

  if (indicator->priv->indicator != NULL)
    {
      g_object_unref (indicator->priv->indicator);
      indicator->priv->indicator = NULL;
    }

  G_OBJECT_CLASS (vino_tube_app_indicator_parent_class)->finalize (object);
}

void
vino_tube_app_indicator_update_state (VinoTubeAppIndicator *indicator)
{
  char     *tooltip;
  gboolean visible;
  AppIndicatorStatus status;

  g_return_if_fail (VINO_IS_TUBE_APP_INDICATOR (indicator));

  visible = !vino_server_get_on_hold (VINO_SERVER (indicator->priv->server));

  tooltip = g_strdup (_("Desktop sharing is enabled"));

  visible = visible && (indicator->priv->visibility == VINO_STATUS_TUBE_ICON_VISIBILITY_ALWAYS);

  gtk_menu_item_set_label (GTK_MENU_ITEM (indicator->priv->status_menu_item), tooltip);

  status = visible ? APP_INDICATOR_STATUS_ACTIVE : APP_INDICATOR_STATUS_PASSIVE;
  app_indicator_set_status (indicator->priv->indicator, status);

  g_free (tooltip);
}


static void
vino_tube_app_indicator_create_menu (VinoTubeAppIndicator *indicator)
{
  GtkWidget *item;
  char *str;

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
      G_CALLBACK (vino_tube_app_indicator_preferences), indicator);
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  item = gtk_separator_menu_item_new ();
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  /* Translators: %s is the alias of the telepathy contact */
  if (indicator->priv->server != NULL) {
    str = g_strdup_printf (_("Disconnect %s"),
      vino_tube_server_get_alias (indicator->priv->server));
  } else {
    str = g_strdup_printf (_("Disconnect"));
  }

  item  = gtk_image_menu_item_new_with_label (str);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
  gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU));

  g_signal_connect_swapped (item, "activate",
      G_CALLBACK (vino_tube_app_indicator_disconnect_confirm), indicator);

  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  g_free (str);

  item = gtk_separator_menu_item_new ();
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);

  item = gtk_image_menu_item_new_with_mnemonic (_("_Help"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
      gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU));
  g_signal_connect_swapped (item, "activate",
      G_CALLBACK (vino_tube_app_indicator_help), indicator);
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (indicator->priv->menu), item);
  app_indicator_set_menu (indicator->priv->indicator, indicator->priv->menu);
}

static void
vino_tube_app_indicator_init (VinoTubeAppIndicator *indicator)
{
  indicator->priv = G_TYPE_INSTANCE_GET_PRIVATE (indicator, VINO_TYPE_TUBE_APP_INDICATOR, VinoTubeAppIndicatorPrivate);

  indicator->priv->indicator = app_indicator_new ("vino-tube-app-indicator",
						  "preferences-desktop-remote-desktop",
						  APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  vino_tube_app_indicator_create_menu (indicator);
  app_indicator_set_menu (indicator->priv->indicator, indicator->priv->menu);

#ifdef VINO_ENABLE_LIBNOTIFY
  indicator->priv->new_client_notification = NULL;
#endif
}

static void
vino_tube_app_indicator_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
  VinoTubeAppIndicator *indicator = VINO_TUBE_APP_INDICATOR (object);

  switch (prop_id)
    {
    case PROP_SERVER:
      indicator->priv->server = g_value_get_object (value);
      break;
    case PROP_VISIBILITY:
      vino_tube_app_indicator_set_visibility (indicator, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vino_tube_app_indicator_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
  VinoTubeAppIndicator *indicator = VINO_TUBE_APP_INDICATOR (object);

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

VinoTubeAppIndicator*
vino_tube_app_indicator_new (VinoTubeServer *server)
{
  g_return_val_if_fail (VINO_IS_TUBE_SERVER (server), NULL);

  return g_object_new (VINO_TYPE_TUBE_APP_INDICATOR, "server", server, NULL);
}

static void
vino_tube_app_indicator_preferences (VinoTubeAppIndicator *indicator)
{
  GdkScreen *screen;
  GDesktopAppInfo *info;
  GdkAppLaunchContext *context;
  GError *error = NULL;

  screen = gdk_screen_get_default ();
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
vino_tube_app_indicator_help (VinoTubeAppIndicator *indicator)
{
  GdkScreen *screen;
  GError    *error = NULL;

  screen = gdk_screen_get_default ();
  if (!gtk_show_uri (screen, "ghelp:user-guide?goscustdesk-90",
      GDK_CURRENT_TIME, &error))
    {
      vino_util_show_error (_("Error displaying help"), error->message, NULL);
      g_error_free (error);
    }
}

static void
vino_tube_app_indicator_disconnect_client (VinoTubeAppIndicator *indicator,
    gint response)
{
  gtk_widget_destroy (indicator->priv->disconnect_dialog);
  indicator->priv->disconnect_dialog = NULL;

  if (response == GTK_RESPONSE_OK)
    {
      vino_tube_server_close_tube (indicator->priv->server);
    }
}

static void
vino_tube_app_indicator_disconnect_confirm (VinoTubeAppIndicator *indicator)
{
  char      *primary_msg;
  char      *secondary_msg;

  if (indicator->priv->disconnect_dialog)
  {
    gtk_window_present (GTK_WINDOW (indicator->priv->disconnect_dialog));
    return;
  }

  /* Translators: %s is the alias of the telepathy contact */
  primary_msg   = g_strdup_printf
      (_("Are you sure you want to disconnect '%s'?"),
      vino_tube_server_get_alias (indicator->priv->server));
  secondary_msg = g_strdup_printf
      (_("The remote user '%s' will be disconnected. Are you sure?"),
      vino_tube_server_get_alias (indicator->priv->server));

  indicator->priv->disconnect_dialog = gtk_message_dialog_new (NULL,
      GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_CANCEL, "%s", primary_msg);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW
      (indicator->priv->disconnect_dialog), FALSE);

  gtk_dialog_add_button (GTK_DIALOG (indicator->priv->disconnect_dialog),
      _("Disconnect"), GTK_RESPONSE_OK);

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG
      (indicator->priv->disconnect_dialog), "%s", secondary_msg);

  g_signal_connect_swapped (indicator->priv->disconnect_dialog, "response",
      G_CALLBACK (vino_tube_app_indicator_disconnect_client),
      indicator);

  gtk_widget_show_all (GTK_WIDGET (indicator->priv->disconnect_dialog));

  g_free (primary_msg);
  g_free (secondary_msg);

}

static void
vino_tube_app_indicator_class_init (VinoTubeAppIndicatorClass *klass)
{
  GObjectClass       *gobject_class     = G_OBJECT_CLASS (klass);

  gobject_class->finalize     = vino_tube_app_indicator_finalize;
  gobject_class->set_property = vino_tube_app_indicator_set_property;
  gobject_class->get_property = vino_tube_app_indicator_get_property;

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
      "Icon visibility",
      "When the icon must be shown",
      VINO_TYPE_STATUS_ICON_VISIBILITY,
      VINO_STATUS_ICON_VISIBILITY_CLIENT,
      G_PARAM_READWRITE |
      G_PARAM_CONSTRUCT |
      G_PARAM_STATIC_NAME |
      G_PARAM_STATIC_NICK |
      G_PARAM_STATIC_BLURB));

  g_type_class_add_private (gobject_class, sizeof (VinoTubeAppIndicatorPrivate));
}

void
vino_tube_app_indicator_set_visibility (VinoTubeAppIndicator *indicator,
    VinoStatusTubeIconVisibility  visibility)
{
  g_return_if_fail (VINO_IS_TUBE_APP_INDICATOR (indicator));
  g_return_if_fail (visibility != VINO_STATUS_TUBE_ICON_VISIBILITY_INVALID);

  if (visibility != indicator->priv->visibility)
    {
      indicator->priv->visibility = visibility;
      vino_tube_app_indicator_update_state (indicator);
    }
}

#ifdef VINO_ENABLE_LIBNOTIFY
static void
vino_tube_app_indicator_show_invalidated_notif_closed
    (VinoStatusTubeAppIndicator *indicator)
{
  dprintf (TUBE, "Notification was closed");
  vino_tube_server_fire_closed (indicator->priv->server);
}
#endif

void
vino_tube_app_indicator_show_notif (VinoTubeAppIndicator *indicator,
    const gchar *summary, const gchar *body, gboolean invalidated)
{
#ifdef VINO_ENABLE_LIBNOTIFY
#define NOTIFICATION_TIMEOUT 5

  GError *error;
  const gchar *filename = NULL;

  if (!notify_is_initted () &&  !notify_init (g_get_application_name ()))
    {
      g_printerr (_("Error initializing libnotify\n"));
      return;
    }

  if (indicator->priv->new_client_notification != NULL)
    {
      notify_notification_close (indicator->priv->new_client_notification, NULL);
      g_object_unref (indicator->priv->new_client_notification);
      indicator->priv->new_client_notification = NULL;
    }

  filename = vino_tube_server_get_avatar_filename (indicator->priv->server);

  if (filename == NULL)
      filename = "stock_person";

  indicator->priv->new_client_notification =
	  notify_notification_new (summary, body, filename);

  notify_notification_set_timeout (indicator->priv->new_client_notification,
      NOTIFICATION_TIMEOUT * 1000);

  if (invalidated)
    g_signal_connect_swapped (indicator->priv->new_client_notification, "closed",
        G_CALLBACK (vino_tube_app_indicator_show_invalidated_notif_closed),
        indicator);

  error = NULL;
  if (!notify_notification_show (indicator->priv->new_client_notification, &error))
    {
      g_printerr (_("Error while displaying notification bubble: %s\n"),
                  error->message);
      g_error_free (error);
    }

#undef NOTIFICATION_TIMEOUT
#else
  if (invalidated)
    vino_tube_server_fire_closed (indicator->priv->server);
#endif /* VINO_ENABLE_LIBNOTIFY */
}

