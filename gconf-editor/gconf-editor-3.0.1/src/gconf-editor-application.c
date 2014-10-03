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
#include "gconf-editor-application.h"

#include "gconf-util.h"
#include "gconf-editor-window.h"
#include "gconf-tree-model.h"
#include "gconf-list-model.h"
#include <gtk/gtk.h>


static GSList *editor_windows = NULL;

static void
gconf_editor_application_window_destroyed (GtkWidget *window)
{
	editor_windows = g_slist_remove (editor_windows, window);

	if (editor_windows == NULL)
		gtk_main_quit ();
}

GtkWidget *
gconf_editor_application_create_editor_window (int type)
{
	GtkWidget *window;
	GConfEditorWindow *gconfwindow;

	window = g_object_new (GCONF_TYPE_EDITOR_WINDOW, "editor-type", type, NULL);

	gconfwindow = GCONF_EDITOR_WINDOW (window);

	if (gconfwindow->client == NULL) {
		gconf_editor_application_window_destroyed (window);
		return NULL;
	}

	gconf_tree_model_set_client (GCONF_TREE_MODEL (gconfwindow->tree_model), gconfwindow->client);
	gconf_list_model_set_client (GCONF_LIST_MODEL (gconfwindow->list_model), gconfwindow->client);

	if (!gconf_util_can_edit_defaults ())
		gtk_action_set_sensitive (gtk_ui_manager_get_action (gconfwindow->ui_manager, "/GConfEditorMenu/FileMenu/NewDefaultsWindow"),
					  FALSE);

	if (!gconf_util_can_edit_mandatory ())
		gtk_action_set_sensitive (gtk_ui_manager_get_action (gconfwindow->ui_manager, "/GConfEditorMenu/FileMenu/NewMandatoryWindow"),
					  FALSE);
	
	g_signal_connect (window, "destroy",
			  G_CALLBACK (gconf_editor_application_window_destroyed), NULL);
	
	editor_windows = g_slist_prepend (editor_windows, window);

	gconf_editor_window_expand_first (gconfwindow);

	return window;
}

