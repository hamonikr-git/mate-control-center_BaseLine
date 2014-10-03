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

#include <config.h>

#include "vino-prompt.h"

#include <gtk/gtk.h>
#include <libnotify/notify.h>
#include "vino-util.h"
#include "vino-enums.h"
#include "vino-marshal.h"

struct _VinoPromptPrivate
{
  GdkScreen          *screen;
  NotifyNotification *notification;
  rfbClientPtr        current_client;
  GSList             *pending_clients;
};

enum
{
  PROP_0,
  PROP_SCREEN
};

enum
{
  RESPONSE,
  LAST_SIGNAL
};

static gboolean vino_prompt_display (VinoPrompt   *prompt,
				     rfbClientPtr  rfb_client);

static guint prompt_signals [LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (VinoPrompt, vino_prompt, G_TYPE_OBJECT);

static void
clear_notification (VinoPrompt *prompt)
{
  if (prompt->priv->notification)
    notify_notification_close (prompt->priv->notification, NULL);
  g_clear_object (&prompt->priv->notification);
}

static void
vino_prompt_finalize (GObject *object)
{
  VinoPrompt *prompt = VINO_PROMPT (object);

  g_slist_free (prompt->priv->pending_clients);
  prompt->priv->pending_clients = NULL;

  clear_notification (prompt);

  g_free (prompt->priv);
  prompt->priv = NULL;

  if (G_OBJECT_CLASS (vino_prompt_parent_class)->finalize)
    G_OBJECT_CLASS (vino_prompt_parent_class)->finalize (object);
}

static void
vino_prompt_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  VinoPrompt *prompt = VINO_PROMPT (object);

  switch (prop_id)
    {
    case PROP_SCREEN:
      vino_prompt_set_screen (prompt, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vino_prompt_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  VinoPrompt *prompt = VINO_PROMPT (object);

  switch (prop_id)
    {
    case PROP_SCREEN:
      g_value_set_object (value, prompt->priv->screen);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vino_prompt_init (VinoPrompt *prompt)
{
  prompt->priv = g_new0 (VinoPromptPrivate, 1);
}

static void
vino_prompt_class_init (VinoPromptClass *klass)
{
  GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
  VinoPromptClass *prompt_class  = VINO_PROMPT_CLASS (klass);
  
  gobject_class->finalize     = vino_prompt_finalize;
  gobject_class->set_property = vino_prompt_set_property;
  gobject_class->get_property = vino_prompt_get_property;

  prompt_class->response = NULL;
  
  g_object_class_install_property (gobject_class,
				   PROP_SCREEN,
				   g_param_spec_object ("screen",
							_("Screen"),
							_("The screen on which to display the prompt"),
							GDK_TYPE_SCREEN,
							G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  prompt_signals [RESPONSE] =
    g_signal_new ("response",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (VinoPromptClass, response),
                  NULL, NULL,
                  vino_marshal_VOID__POINTER_ENUM,
                  G_TYPE_NONE,
		  2,
		  G_TYPE_POINTER,
		  VINO_TYPE_PROMPT_RESPONSE);

  vino_init_stock_items ();
}

VinoPrompt *
vino_prompt_new (GdkScreen *screen)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  return g_object_new (VINO_TYPE_PROMPT,
		       "screen", screen,
		       NULL);
}

GdkScreen *
vino_prompt_get_screen (VinoPrompt *prompt)
{
  g_return_val_if_fail (VINO_IS_PROMPT (prompt), NULL);

  return prompt->priv->screen;
}

void
vino_prompt_set_screen (VinoPrompt *prompt,
			GdkScreen  *screen)
{
  g_return_if_fail (VINO_IS_PROMPT (prompt));

  if (prompt->priv->screen != screen)
    {
      prompt->priv->screen = screen;

      g_object_notify (G_OBJECT (prompt), "screen");
    }
}

static void
vino_prompt_process_pending_clients (VinoPrompt *prompt)
{
  if (prompt->priv->pending_clients)
    {
      rfbClientPtr rfb_client = (rfbClientPtr) prompt->priv->pending_clients->data;

      prompt->priv->pending_clients =
	g_slist_delete_link (prompt->priv->pending_clients,
			     prompt->priv->pending_clients);

      vino_prompt_display (prompt, rfb_client);
    }
}

static void
emit_response_signal (VinoPrompt   *prompt,
		      rfbClientPtr  rfb_client,
		      int           response)
{
  dprintf (PROMPT, "Emiting response signal for %p: %s\n",
	   rfb_client,
	   response == VINO_RESPONSE_ACCEPT ? "accept" : "reject");

  g_signal_emit (prompt,
		 prompt_signals [RESPONSE],
		 0,
		 rfb_client,
		 response);
}

static void
vino_prompt_handle_response (NotifyNotification *notification,
			     char               *response,
			     gpointer            user_data)
{
  VinoPrompt *prompt = user_data;
  rfbClientPtr rfb_client;
  int          prompt_response = VINO_RESPONSE_INVALID;

  dprintf (PROMPT, "Got a response for client %p: %s\n",
	   prompt->priv->current_client,
	   response);

  if (g_strcmp0 (response, "accept") == 0)
    prompt_response = VINO_RESPONSE_ACCEPT;
  else
    prompt_response = VINO_RESPONSE_REJECT;

  rfb_client = prompt->priv->current_client;
  prompt->priv->current_client = NULL;

  clear_notification (prompt);

  if (rfb_client != NULL)
    {
      emit_response_signal (prompt, rfb_client, prompt_response);
    }

  vino_prompt_process_pending_clients (prompt);
}

static gboolean
vino_prompt_setup_dialog (VinoPrompt *prompt)
{
  if (!notify_is_initted () &&  !notify_init (g_get_application_name ()))
    {
      g_printerr (_("Error initializing libnotify\n"));
      return FALSE;
    }

  return TRUE;
}

static gboolean
vino_prompt_display (VinoPrompt   *prompt,
		     rfbClientPtr  rfb_client)
{
  char *host_label;

  if (prompt->priv->current_client)
    return prompt->priv->current_client == rfb_client;

  if (!vino_prompt_setup_dialog (prompt))
    return FALSE;

  host_label = g_strdup_printf (_("A user on the computer '%s' is trying to remotely view or control your desktop."),
				rfb_client->host);

  prompt->priv->notification = notify_notification_new (_("Another user is trying to view your desktop."),
							host_label,
							"preferences-desktop-remote-desktop");
  notify_notification_set_hint_string (prompt->priv->notification, "desktop-entry", "vino-server");
  notify_notification_add_action (prompt->priv->notification,
				  "refuse",
				  _("Refuse"),
				  vino_prompt_handle_response,
				  prompt,
				  NULL);
  notify_notification_add_action (prompt->priv->notification,
				  "accept",
				  _("Accept"),
				  vino_prompt_handle_response,
				  prompt,
				  NULL);

  g_free (host_label);

  prompt->priv->current_client = rfb_client;

  notify_notification_show (prompt->priv->notification, NULL);

  dprintf (PROMPT, "Prompting for client %p\n", rfb_client);

  return TRUE;
}

void
vino_prompt_add_client (VinoPrompt   *prompt,
			rfbClientPtr  rfb_client)
{
  g_return_if_fail (VINO_IS_PROMPT (prompt));
  g_return_if_fail (rfb_client != NULL);

  if (!vino_prompt_display (prompt, rfb_client))
    {
      dprintf (PROMPT, "Prompt in progress for %p: queueing %p\n",
	       prompt->priv->current_client, rfb_client);
      prompt->priv->pending_clients =
	g_slist_append (prompt->priv->pending_clients, rfb_client);
    }
}

void
vino_prompt_remove_client (VinoPrompt   *prompt,
			   rfbClientPtr  rfb_client)
{
  g_return_if_fail (VINO_IS_PROMPT (prompt));
  g_return_if_fail (rfb_client != NULL);

  if (prompt->priv->current_client == rfb_client)
    {
      clear_notification (prompt);
    }
  else
    {
      prompt->priv->pending_clients =
	g_slist_remove (prompt->priv->pending_clients, rfb_client);
    }
}
