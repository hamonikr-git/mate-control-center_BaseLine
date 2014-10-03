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

#ifndef __GCONF_LIST_MODEL_H__
#define __GCONF_LIST_MODEL_H__

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#define GCONF_TYPE_LIST_MODEL		  (gconf_list_model_get_type ())
#define GCONF_LIST_MODEL(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCONF_TYPE_LIST_MODEL, GConfListModel))
#define GCONF_LIST_MODEL_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCONF_TYPE_LIST_MODEL, GConfListModelClass))
#define GCONF_IS_LIST_MODEL(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCONF_TYPE_LIST_MODEL))
#define GCONF_IS_LIST_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_LIST_MODEL))
#define GCONF_LIST_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GCONF_TYPE_LIST_MODEL, GConfListModelClass))

typedef struct _GConfListModel GConfListModel;
typedef struct _GConfListModelClass GConfListModelClass;

enum {
	GCONF_LIST_MODEL_ICON_NAME_COLUMN,
	GCONF_LIST_MODEL_KEY_NAME_COLUMN,
	GCONF_LIST_MODEL_KEY_PATH_COLUMN,
	GCONF_LIST_MODEL_VALUE_COLUMN,
	GCONF_LIST_MODEL_NUM_COLUMNS
};

struct _GConfListModel {
	GObject parent_instance;

	gchar *root_path;
	gint stamp;

	GConfClient *client;
	
	GSList *values;
	gint length;

	guint notify_id;
	GHashTable *key_hash;
};

struct _GConfListModelClass {
	GObjectClass parent_class;
};

GType gconf_list_model_get_type (void);
GtkTreeModel *gconf_list_model_new (void);
void gconf_list_model_set_root_path (GConfListModel *model, const gchar *path);
void gconf_list_model_set_client (GConfListModel *model, GConfClient *client);

#endif /* __GCONF_LIST_MODEL_H__ */
