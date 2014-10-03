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

#include "gconf-list-model.h"

#include <string.h>
#include "gconf-util.h"

#define G_SLIST(x) ((GSList *) x)

static const char *gconf_value_icon_names[] = {
  "type-undefined",
  "type-string",
  "type-integer",
  "type-float",
  "type-boolean",
  "type-schema",
  "type-list",
  "type-pair"
};

static void gconf_list_model_class_init       (GConfListModelClass *klass);
static void gconf_list_model_init             (GConfListModel *model);
static void gconf_list_model_tree_model_init  (GtkTreeModelIface *iface);

G_DEFINE_TYPE_WITH_CODE (GConfListModel, gconf_list_model, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, gconf_list_model_tree_model_init));

static void
gconf_list_model_notify_func (GConfClient* client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GSList *list;
	const gchar *key;
	char *path_str;
	GConfListModel *list_model = user_data;
	GtkTreeIter iter;
	GtkTreePath *path;

	key = gconf_entry_get_key (entry);

	path_str = g_path_get_dirname (key);

	if (strcmp (path_str, list_model->root_path) != 0)
	  {
	    g_free (path_str);
	    return;
	  }

	g_free (path_str);

	if (strncmp (key, list_model->root_path, strlen (list_model->root_path)) != 0)
	    return;
	
	if (gconf_client_dir_exists (client, key, NULL))
		/* this is a directory -- ignore */
		return;

	list = g_hash_table_lookup (list_model->key_hash, key);

	if (list == NULL) {
		/* Create a new entry */
		entry = gconf_entry_new (gconf_entry_get_key (entry),
					 gconf_entry_get_value (entry));

		list = g_slist_append (list, entry);
		list_model->values = g_slist_concat (list_model->values, list);
		g_hash_table_insert (list_model->key_hash, g_strdup (key), list);

		list_model->stamp++;

		iter.stamp = list_model->stamp;
		iter.user_data = list;

		list_model->length++;

		path = gtk_tree_model_get_path (GTK_TREE_MODEL (list_model), &iter);
		gtk_tree_model_row_inserted (GTK_TREE_MODEL (list_model), path, &iter);
		gtk_tree_path_free (path);
	}
	else {
		list_model->stamp++;

		iter.stamp = list_model->stamp;
		iter.user_data = list;

		path = gtk_tree_model_get_path (GTK_TREE_MODEL (list_model), &iter);

		gconf_entry_unref (list->data);

		if (gconf_entry_get_value (entry) != NULL) {
			list->data = gconf_entry_new (gconf_entry_get_key (entry),
						      gconf_entry_get_value (entry));
			gtk_tree_model_row_changed (GTK_TREE_MODEL (list_model), path, &iter);
		}
		else {
			gtk_tree_model_row_deleted (GTK_TREE_MODEL (list_model), path);
			list_model->values = g_slist_remove (list_model->values, list->data);
			list_model->length--;
			g_hash_table_remove (list_model->key_hash, key);
		}

		gtk_tree_path_free (path);
	}
}

void
gconf_list_model_set_root_path (GConfListModel *model, const gchar *root_path)
{
	GSList *list;
	GSList *values;
	GtkTreeIter iter;
	GtkTreePath *path;

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, 0);

	if (model->root_path != NULL) {
		for (list = model->values; list; list = list->next) {
			GConfEntry *entry = list->data;

			g_hash_table_remove (model->key_hash, gconf_entry_get_key (entry));
			model->stamp++;
			gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);

			gconf_entry_unref (entry);
		}

		gconf_client_notify_remove (model->client, model->notify_id);

		gconf_client_remove_dir  (model->client,
					  model->root_path, NULL);

		g_free (model->root_path);
		g_slist_free (model->values);
		model->values = NULL;
	}
	gtk_tree_path_free (path);

	gconf_client_add_dir (model->client,
			      root_path,
			      GCONF_CLIENT_PRELOAD_ONELEVEL,
			      NULL);

	model->notify_id = gconf_client_notify_add (model->client, root_path,
						    gconf_list_model_notify_func,
						    model, NULL, NULL);

	model->root_path = g_strdup (root_path);
	values = gconf_client_all_entries (model->client, root_path, NULL);
	model->length = 0;

	for (list = values; list; list = list->next) {
		GConfEntry *entry = list->data;

		model->values = g_slist_append (model->values, list->data);
		model->length++;

		model->stamp++;

		iter.stamp = model->stamp;
		iter.user_data = g_slist_last (model->values);

		g_hash_table_insert (model->key_hash, g_strdup (gconf_entry_get_key (entry)), iter.user_data);

		path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
		gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), path, &iter);

		gtk_tree_path_free (path);
	}

	if (values) 
		g_slist_free (values);
}

static GtkTreeModelFlags 
gconf_list_model_get_flags (GtkTreeModel *tree_model)
{
	return 0;
}

static gint
gconf_list_model_get_n_columns (GtkTreeModel *tree_model)
{
	return GCONF_LIST_MODEL_NUM_COLUMNS;
}

static GType
gconf_list_model_get_column_type (GtkTreeModel *tree_model, gint index)
{
	switch (index) {
	case GCONF_LIST_MODEL_ICON_NAME_COLUMN:
	case GCONF_LIST_MODEL_KEY_PATH_COLUMN:
	case GCONF_LIST_MODEL_KEY_NAME_COLUMN:
		return G_TYPE_STRING;
	case GCONF_LIST_MODEL_VALUE_COLUMN:
		return GCONF_TYPE_VALUE;
	default:
		return G_TYPE_INVALID;
	}
}

static gboolean
gconf_list_model_get_iter (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path)
{
	GConfListModel *list_model = (GConfListModel *)tree_model;
	GSList *list;
	gint i;

	g_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

	i = gtk_tree_path_get_indices (path)[0];

	if (i >= list_model->length)
		return FALSE;

	list = g_slist_nth (list_model->values, i);

	iter->stamp = list_model->stamp;
	iter->user_data = list;

	return TRUE;
}

static GtkTreePath *
gconf_list_model_get_path (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	GSList *list;
	GtkTreePath *tree_path;
	gint i = 0;

	g_return_val_if_fail (iter->stamp == GCONF_LIST_MODEL (tree_model)->stamp, NULL);

	for (list = G_SLIST (GCONF_LIST_MODEL (tree_model)->values); list; list = list->next) {
		if (list == G_SLIST (iter->user_data))
			break;
		i++;
	}
	if (list == NULL)
		return NULL;

	tree_path = gtk_tree_path_new ();
	gtk_tree_path_append_index (tree_path, i);

	return tree_path;
}

static void
gconf_list_model_get_value (GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value)
{
	GConfEntry *entry;
	
	g_return_if_fail (iter->stamp == GCONF_LIST_MODEL (tree_model)->stamp);

	entry = G_SLIST (iter->user_data)->data;
	
	switch (column) {
	case GCONF_LIST_MODEL_KEY_PATH_COLUMN:
		g_value_init (value, G_TYPE_STRING);
		g_value_set_string (value, gconf_entry_get_key (entry));
		break;
		
	case GCONF_LIST_MODEL_KEY_NAME_COLUMN:
		g_value_init (value, G_TYPE_STRING);
		g_value_take_string (value, gconf_get_key_name_from_path (gconf_entry_get_key (entry)));
		break;
		
	case GCONF_LIST_MODEL_ICON_NAME_COLUMN: {
                GConfValue *gconf_value;
                GConfValueType value_type;

                gconf_value = gconf_entry_get_value (entry);
                if (gconf_value) {
                        value_type = gconf_value->type;
                } else {
                        value_type = GCONF_VALUE_INVALID;
                }

                g_value_init (value, G_TYPE_STRING);
                g_value_set_static_string (value, gconf_value_icon_names[value_type]);
		
		break;
        }
		
	case GCONF_LIST_MODEL_VALUE_COLUMN:
		g_value_init (value, GCONF_TYPE_VALUE);
		g_value_set_boxed (value, gconf_entry_get_value (entry));
		break;

	default:
		break;
	}
}

static gboolean
gconf_list_model_iter_next (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	g_return_val_if_fail (iter->stamp == GCONF_LIST_MODEL (tree_model)->stamp, FALSE);

	iter->user_data = G_SLIST (iter->user_data)->next;
	
	return (iter->user_data != NULL);
}

static gboolean
gconf_list_model_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	return FALSE;
}

static gint
gconf_list_model_iter_n_children (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	/* it should ask for the root node, because we're a list */
	if (!iter)
		return g_slist_length (GCONF_LIST_MODEL (tree_model)->values);

	return -1;
}

static gboolean
gconf_list_model_iter_nth_child (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	GSList *child;

	if (parent)
		return FALSE;
	
	child = g_slist_nth (GCONF_LIST_MODEL (tree_model)->values, n);

	if (child) {
		iter->stamp = GCONF_LIST_MODEL (tree_model)->stamp;
		iter->user_data = child;
		return TRUE;
	}
	else
		return FALSE;
}

static void
gconf_list_model_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags = gconf_list_model_get_flags;
	iface->get_n_columns = gconf_list_model_get_n_columns;
	iface->get_column_type = gconf_list_model_get_column_type;
	iface->get_iter = gconf_list_model_get_iter;
	iface->get_path = gconf_list_model_get_path;
	iface->get_value = gconf_list_model_get_value;
	iface->iter_next = gconf_list_model_iter_next;
	iface->iter_has_child = gconf_list_model_iter_has_child;
	iface->iter_n_children = gconf_list_model_iter_n_children;
	iface->iter_nth_child = gconf_list_model_iter_nth_child;
}

static void
gconf_list_model_finalize (GObject *object)
{
	GConfListModel *list_model;

	list_model = (GConfListModel *)object;
	
	g_hash_table_destroy (list_model->key_hash);

	if (list_model->client && list_model->notify_id > 0) {
		gconf_client_notify_remove (list_model->client, list_model->notify_id);
	}

	if (list_model->client && list_model->root_path) {
		gconf_client_remove_dir  (list_model->client,
					  list_model->root_path, NULL);
	}

        if (list_model->client) {
                g_object_unref (list_model->client);
		list_model->client = NULL;
        }

	if (list_model->values) {
		g_slist_foreach (list_model->values, (GFunc) gconf_entry_unref, NULL);
		g_slist_free (list_model->values);
		list_model->values = NULL;
	}

	g_free (list_model->root_path);

	G_OBJECT_CLASS (gconf_list_model_parent_class)->finalize (object);
}

static void
gconf_list_model_class_init (GConfListModelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gconf_list_model_finalize;
}

static void
gconf_list_model_init (GConfListModel *model)
{
	model->stamp = g_random_int ();
	model->key_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
						 g_free, NULL);
	model->client = gconf_client_get_default ();
}

void
gconf_list_model_set_client (GConfListModel *model, GConfClient *client)
{
	if (model->client != NULL) {
		g_object_unref (model->client);
	}

	model->client = g_object_ref (client);
}

GtkTreeModel *
gconf_list_model_new (void)
{
	return GTK_TREE_MODEL (g_object_new (GCONF_TYPE_LIST_MODEL, NULL));
}
