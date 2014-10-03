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

#ifndef __GCONF_TREE_MODEL_H__
#define __GCONF_TREE_MODEL_H__

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#define GCONF_TYPE_TREE_MODEL		  (gconf_tree_model_get_type ())
#define GCONF_TREE_MODEL(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCONF_TYPE_TREE_MODEL, GConfTreeModel))
#define GCONF_TREE_MODEL_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCONF_TYPE_TREE_MODEL, GConfTreeModelClass))
#define GCONF_IS_TREE_MODEL(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCONF_TYPE_TREE_MODEL))
#define GCONF_IS_TREE_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_TREE_MODEL))
#define GCONF_TREE_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GCONF_TYPE_TREE_MODEL, GConfTreeModelClass))

typedef struct _GConfTreeModel GConfTreeModel;
typedef struct _GConfTreeModelClass GConfTreeModelClass;

enum {
	GCONF_TREE_MODEL_NAME_COLUMN,
	GCONF_TREE_MODEL_NUM_COLUMNS
};

struct _GConfTreeModel {
	GObject parent_instance;

	gpointer root;
	gint stamp;

	GConfClient *client;
};

struct _GConfTreeModelClass {
	GObjectClass parent_class;
};


	
GType gconf_tree_model_get_type (void);
GtkTreeModel *gconf_tree_model_new (void);
gchar *gconf_tree_model_get_gconf_name (GConfTreeModel *tree_model, GtkTreeIter *iter);
gchar *gconf_tree_model_get_gconf_path (GConfTreeModel *tree_model, GtkTreeIter *iter);

GtkTreePath *gconf_tree_model_get_tree_path_from_gconf_path (GConfTreeModel *tree_model, const char *path);
void gconf_tree_model_set_client (GConfTreeModel *model, GConfClient *client);

#endif /* __GCONF_TREE_MODEL_H__ */
