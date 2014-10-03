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

#include "gconf-bookmarks.h"

#include "gconf-stock-icons.h"
#include "gconf-tree-model.h"
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <string.h>


static void
gconf_bookmarks_bookmark_activated (GtkWidget *menuitem, GConfEditorWindow *window)
{
	gconf_editor_window_go_to (window,
				   g_object_get_data (G_OBJECT (menuitem), "gconf-key"));
}

static void
gconf_bookmarks_set_item_has_icon (GtkWidget *item,
				   gboolean   have_icons)
{
	GtkWidget *image;

	image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (item));
	if (image && !g_object_get_data (G_OBJECT (item), "gconf-editor-icon"))
		g_object_set_data_full (G_OBJECT (item), "gconf-editor-icon",
					g_object_ref (image), g_object_unref);

	if (!image)
		image = g_object_get_data (G_OBJECT (item), "gconf-editor-icon");

	if (!image)
		return;

	if (have_icons)
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	else
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), NULL);
}

static void
gconf_bookmarks_set_have_icons (GtkWidget *menu, gboolean have_icons)
{
        GList *items, *n;

        items = gtk_container_get_children (GTK_CONTAINER (menu));

        for (n = items; n != NULL; n = n->next) 
                if (GTK_IS_IMAGE_MENU_ITEM (n->data))
                        gconf_bookmarks_set_item_has_icon (GTK_WIDGET (n->data), have_icons);
}

static void
gconf_bookmarks_have_icons_notify (GConfClient       *client,
				   guint              cnxn_id,
				   GConfEntry        *entry,
				   gpointer           data)
{
        GtkWidget *menu;
	gboolean have_icons;

        menu = GTK_WIDGET (data);

	if (entry->value->type != GCONF_VALUE_BOOL)
		return;

	have_icons = gconf_value_get_bool (entry->value);

	gconf_bookmarks_set_have_icons (menu, have_icons);
}

static void
gconf_bookmarks_update_menu (GtkWidget *menu)
{
	GSList *list, *tmp;
	GtkWidget *menuitem, *window;
	GConfClient *client;

	window = g_object_get_data (G_OBJECT (menu), "editor-window");
	client = gconf_client_get_default ();
	
	/* Get the old list and then set it */
	list = gconf_client_get_list (client,
				     "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, NULL);

	if (list != NULL) {
		menuitem = gtk_separator_menu_item_new ();
		gtk_widget_show (menuitem);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	}

	for (tmp = list; tmp; tmp = tmp->next) {
		menuitem = gtk_image_menu_item_new_with_label (tmp->data);
		g_signal_connect (menuitem, "activate",
				  G_CALLBACK (gconf_bookmarks_bookmark_activated), window);
		g_object_set_data_full (G_OBJECT (menuitem), "gconf-key", g_strdup (tmp->data), g_free);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock (STOCK_BOOKMARK,
													 GTK_ICON_SIZE_MENU));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
		gtk_widget_show_all (menuitem);

		g_free (tmp->data);
	}

	g_object_unref (client);
	g_slist_free (list);
}

static void
gconf_bookmarks_key_changed (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GList *child_list, *tmp;
	GtkWidget *menu_item;
	
	child_list = gtk_container_get_children (GTK_CONTAINER (user_data));

	for (tmp = child_list; tmp; tmp = tmp->next) {
		menu_item = tmp->data;

		if (g_object_get_data (G_OBJECT (menu_item), "gconf-key") != NULL ||
			GTK_IS_SEPARATOR_MENU_ITEM (menu_item)) {
			gtk_widget_destroy (menu_item);
		}
	}

	gconf_bookmarks_update_menu (GTK_WIDGET (user_data));
	
	g_list_free (child_list);
}

void
gconf_bookmarks_add_bookmark (const char *path)
{
	GSList *list, *tmp;
	GConfClient *client;

	client = gconf_client_get_default ();

	/* Get the old list and then set it */
	list = gconf_client_get_list (client,
				     "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, NULL);

	/* FIXME: We need error handling here, also this function leaks memory */

	/* Check that the bookmark hasn't been added already */
	for (tmp = list; tmp; tmp = tmp->next) {
		if (strcmp (tmp->data, path) == 0) {
			g_slist_free (list);
			return;
		}
	}

	/* Append the new bookmark */
	list = g_slist_append (list, g_strdup (path));
	
	gconf_client_set_list (client,
			       "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, list, NULL);
	g_slist_free (list);
	g_object_unref (client);
}

static void
remove_notify_id (gpointer data)
{
	GConfClient *client;

	client = gconf_client_get_default ();
	gconf_client_notify_remove (client, GPOINTER_TO_INT (data));

	g_object_unref (client);
}

void
gconf_bookmarks_hook_up_menu (GConfEditorWindow *window,
			      GtkWidget *menu,
			      GtkWidget *add_bookmark,
			      GtkWidget *edit_bookmarks)
{
	GConfClient *client;
	guint notify_id;

	g_object_set_data (G_OBJECT (menu), "editor-window", window);

	client = gconf_client_get_default ();
	
	/* Add a notify function */
	gconf_client_add_dir (client, "/apps/gconf-editor/bookmarks",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	notify_id = gconf_client_notify_add (client, "/apps/gconf-editor/bookmarks",
					     gconf_bookmarks_key_changed, menu, NULL, NULL);
	g_object_set_data_full (G_OBJECT (menu), "notify-id", GINT_TO_POINTER (notify_id),
				remove_notify_id);


	notify_id = gconf_client_notify_add (client, "/desktop/gnome/interface/menus_have_icons",
					     gconf_bookmarks_have_icons_notify, menu, NULL, NULL); 
	g_object_set_data_full (G_OBJECT (menu), "notify-id-x", GINT_TO_POINTER (notify_id),
				remove_notify_id);

	gconf_bookmarks_update_menu (menu);

        {
                gboolean have_icons;
                GConfValue *value;
                GError *err;

                err = NULL;
                value = gconf_client_get (client, "/desktop/gnome/interface/menus_have_icons", &err);

                if (err != NULL || value == NULL || value->type != GCONF_VALUE_BOOL)
                        return;

                have_icons = gconf_value_get_bool (value);
                gconf_bookmarks_set_have_icons (menu, have_icons);

                gconf_value_free (value);
        }

	if ( ! gconf_client_key_is_writable (client, "/apps/gconf-editor/bookmarks", NULL)) {
		gtk_widget_set_sensitive (add_bookmark, FALSE);
		gtk_widget_set_sensitive (edit_bookmarks, FALSE);
	}

	g_object_unref (client);
}
