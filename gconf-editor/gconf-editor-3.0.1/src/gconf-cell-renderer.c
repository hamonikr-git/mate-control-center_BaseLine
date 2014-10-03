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

#include <config.h>
#include "gconf-cell-renderer.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gconf-util.h"
#include "gconf-marshal.h"

#define GCONF_CELL_RENDERER_TEXT_PATH "gconf-cell-renderer-text-path"
#define GCONF_CELL_RENDERER_VALUE "gconf-cell-renderer-value"
#define SCHEMA_TEXT "<schema>"

enum {
	PROP_ZERO,
	PROP_VALUE
};

enum {
	CHANGED,
	CHECK_WRITABLE,
	LAST_SIGNAL
};

static guint gconf_cell_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(GConfCellRenderer, gconf_cell_renderer, GTK_TYPE_CELL_RENDERER)


static void
gconf_cell_renderer_get_property (GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
	GConfCellRenderer *cellvalue;

	cellvalue = GCONF_CELL_RENDERER (object);
	
	switch (param_id) {
	case PROP_VALUE:
		g_value_set_boxed (value, cellvalue->value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

}

static void
gconf_cell_renderer_set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
	GConfCellRenderer *cellvalue;
	GConfValue *gconf_value;
	GtkCellRendererMode new_mode = GTK_CELL_RENDERER_MODE_INERT;
	
	cellvalue = GCONF_CELL_RENDERER (object);

	switch (param_id) {
	case PROP_VALUE:
		if (cellvalue->value) 
			gconf_value_free (cellvalue->value);

		gconf_value = g_value_get_boxed (value);

		if (gconf_value) {
			cellvalue->value = gconf_value_copy (gconf_value);

			switch (gconf_value->type) {
			case GCONF_VALUE_INT:
			case GCONF_VALUE_FLOAT:
			case GCONF_VALUE_STRING:
				new_mode = GTK_CELL_RENDERER_MODE_EDITABLE;
				break;
				
			case GCONF_VALUE_BOOL:
				new_mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
				break;

			case GCONF_VALUE_LIST:
			case GCONF_VALUE_SCHEMA:
			case GCONF_VALUE_PAIR:
				new_mode = GTK_CELL_RENDERER_MODE_INERT;
				break;
			default:
				g_warning ("unhandled value type %d", gconf_value->type);
				break;
			}
		}
		else {
			cellvalue->value = NULL;
		}

		g_object_set (object, "mode", new_mode, NULL);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;	
	}
}

static void
gconf_cell_renderer_finalize (GObject *object)
{
	GConfCellRenderer *renderer = GCONF_CELL_RENDERER (object);

	if (renderer->value)
		gconf_value_free (renderer->value);

	g_object_unref (renderer->text_renderer);
	g_object_unref (renderer->toggle_renderer);

	(* G_OBJECT_CLASS (gconf_cell_renderer_parent_class)->finalize) (object);
}

static void
gconf_cell_renderer_get_size (GtkCellRenderer *cell,
			      GtkWidget *widget,
			      const GdkRectangle *cell_area,
			      gint *x_offset,
			      gint *y_offset,
			      gint *width,
			      gint *height)
{
	GConfCellRenderer *cellvalue;
	gchar *tmp_str;
	
	cellvalue = GCONF_CELL_RENDERER (cell);

	if (cellvalue->value == NULL) {
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", _("<no value>"),
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer, widget, cell_area,
					    x_offset, y_offset, width, height);
		return;
	}
	
	switch (cellvalue->value->type) {
	case GCONF_VALUE_FLOAT:
	case GCONF_VALUE_INT:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer, widget, cell_area,
					    x_offset, y_offset, width, height);
		g_free (tmp_str);
		break;
	case GCONF_VALUE_STRING:
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", gconf_value_get_string (cellvalue->value),
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer, widget, cell_area,
					    x_offset, y_offset, width, height);
		break;
	case GCONF_VALUE_BOOL:
		g_object_set (G_OBJECT (cellvalue->toggle_renderer),
			      "xalign", 0.0,
			      "active", gconf_value_get_bool (cellvalue->value),
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->toggle_renderer, widget, cell_area,
					    x_offset, y_offset, width, height);
		break;
        case GCONF_VALUE_SCHEMA:
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", SCHEMA_TEXT,
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer,
					    widget, cell_area,
					    x_offset, y_offset, width, height);
		break;
	case GCONF_VALUE_LIST:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer,
					    widget, cell_area,
					    x_offset, y_offset, width, height);
		g_free (tmp_str);
		break;
	case GCONF_VALUE_PAIR:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		gtk_cell_renderer_get_size (cellvalue->text_renderer,
					    widget, cell_area,
					    x_offset, y_offset, width, height);
		g_free (tmp_str);
		break;
	default:
		g_print ("get_size: Unknown type: %d\n", cellvalue->value->type);
		break;
	}
}

static void
gconf_cell_renderer_text_editing_done (GtkCellEditable *entry, GConfCellRenderer *cell)
{
	const gchar *path;
	const gchar *new_text;
	gboolean editing_canceled = FALSE;
	GConfValue *value;

	g_object_get (entry, "editing-canceled", &editing_canceled, NULL);
	if (editing_canceled != FALSE)
		return;

	path = g_object_get_data (G_OBJECT (entry), GCONF_CELL_RENDERER_TEXT_PATH);
	value = g_object_get_data (G_OBJECT (entry), GCONF_CELL_RENDERER_VALUE);
	new_text = gtk_entry_get_text (GTK_ENTRY (entry));

	switch (value->type) {
	case GCONF_VALUE_STRING:
		gconf_value_set_string (value, new_text);
		break;
	case GCONF_VALUE_FLOAT:
		gconf_value_set_float (value, (gdouble)(g_ascii_strtod (new_text, NULL)));
		break;
	case GCONF_VALUE_INT:
		gconf_value_set_int (value, (gint)(g_ascii_strtod (new_text, NULL)));
		break;
	default:
		g_error ("editing done, unknown value %d", value->type);
		break;
	}
	
	g_signal_emit (cell, gconf_cell_signals[CHANGED], 0, path, value);
}

static gboolean
gconf_cell_renderer_check_writability (GConfCellRenderer *cell, const gchar *path)
{
	gboolean ret = TRUE;
	g_signal_emit (cell, gconf_cell_signals[CHECK_WRITABLE], 0, path, &ret);
	return ret;
}

static GtkCellEditable *
gconf_cell_renderer_start_editing (GtkCellRenderer      *cell,
				   GdkEvent             *event,
				   GtkWidget            *widget,
				   const gchar          *path,
				   const GdkRectangle   *background_area,
				   const GdkRectangle   *cell_area,
				   GtkCellRendererState  flags)
{
	GtkWidget *entry;
	GConfCellRenderer *cellvalue;
	gchar *tmp_str;

	cellvalue = GCONF_CELL_RENDERER (cell);

	/* If not writable then we definately can't edit */
	if ( ! gconf_cell_renderer_check_writability (cellvalue, path))
		return NULL;
	
	switch (cellvalue->value->type) {
	case GCONF_VALUE_INT:
	case GCONF_VALUE_FLOAT:
	case GCONF_VALUE_STRING:
		tmp_str = gconf_value_to_string (cellvalue->value);
		entry = g_object_new (GTK_TYPE_ENTRY,
				      "has_frame", FALSE,
				      "text", tmp_str,
				      NULL);
		g_free (tmp_str);
		g_signal_connect (entry, "editing_done",
				  G_CALLBACK (gconf_cell_renderer_text_editing_done), cellvalue);

		g_object_set_data_full (G_OBJECT (entry), GCONF_CELL_RENDERER_TEXT_PATH, g_strdup (path), g_free);
		g_object_set_data_full (G_OBJECT (entry), GCONF_CELL_RENDERER_VALUE, gconf_value_copy (cellvalue->value), (GDestroyNotify)gconf_value_free);
		
		gtk_widget_show (entry);

		return GTK_CELL_EDITABLE (entry);
		break;
	default:
		g_error ("%d shouldn't be handled here", cellvalue->value->type);
		break;
	}

	
	return NULL;
}

static gint
gconf_cell_renderer_activate (GtkCellRenderer      *cell,
			      GdkEvent             *event,
			      GtkWidget            *widget,
			      const gchar          *path,
			      const GdkRectangle   *background_area,
			      const GdkRectangle   *cell_area,
			      GtkCellRendererState  flags)
{
	GConfCellRenderer *cellvalue;
	
	cellvalue = GCONF_CELL_RENDERER (cell);

	if (cellvalue->value == NULL)
		return TRUE;

	switch (cellvalue->value->type) {
	case GCONF_VALUE_BOOL:
		gconf_value_set_bool (cellvalue->value, !gconf_value_get_bool (cellvalue->value));
		g_signal_emit (cell, gconf_cell_signals[CHANGED], 0, path, cellvalue->value);
		
		break;
	default:
		g_error ("%d shouldn't be handled here", cellvalue->value->type);
		break;
	}

	return TRUE;
}

static void
gconf_cell_renderer_render (GtkCellRenderer *cell,
			    cairo_t *cr,
			    GtkWidget *widget,
			    const GdkRectangle *background_area,
			    const GdkRectangle *cell_area,
			    GtkCellRendererState flags)
{
	GConfCellRenderer *cellvalue;
	char *tmp_str;
	
	cellvalue = GCONF_CELL_RENDERER (cell);

	if (cellvalue->value == NULL) {
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", _("<no value>"),
			      NULL);

		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		return;
	}

	switch (cellvalue->value->type) {
	case GCONF_VALUE_FLOAT:
	case GCONF_VALUE_INT:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		g_free (tmp_str);
		break;
	case GCONF_VALUE_STRING:
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", gconf_value_get_string (cellvalue->value),
			      NULL);
		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		break;
	case GCONF_VALUE_BOOL:
		g_object_set (G_OBJECT (cellvalue->toggle_renderer),
			      "xalign", 0.0,
			      "active", gconf_value_get_bool (cellvalue->value),
			      NULL);
		
		gtk_cell_renderer_render (cellvalue->toggle_renderer, cr, widget,
					  background_area, cell_area, flags);
		break;

	case GCONF_VALUE_SCHEMA:
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", SCHEMA_TEXT,
			      NULL);
		
		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		break;
		
	case GCONF_VALUE_LIST:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		
		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		g_free (tmp_str);
		break;
	case GCONF_VALUE_PAIR:
		tmp_str = gconf_value_to_string (cellvalue->value);
		g_object_set (G_OBJECT (cellvalue->text_renderer),
			      "text", tmp_str,
			      NULL);
		gtk_cell_renderer_render (cellvalue->text_renderer, cr, widget,
					  background_area, cell_area, flags);
		g_free (tmp_str);
		break;

	default:
		g_print ("render: Unknown type: %d\n", cellvalue->value->type);
		break;
	}
}

static void
gconf_cell_renderer_class_init (GConfCellRendererClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;
	GtkCellRendererClass *cell_renderer_class = (GtkCellRendererClass *)klass;

	object_class->get_property = gconf_cell_renderer_get_property;
	object_class->set_property = gconf_cell_renderer_set_property;
	object_class->finalize = gconf_cell_renderer_finalize;

	cell_renderer_class->get_size = gconf_cell_renderer_get_size;
	cell_renderer_class->render = gconf_cell_renderer_render;
	cell_renderer_class->activate = gconf_cell_renderer_activate;
	cell_renderer_class->start_editing = gconf_cell_renderer_start_editing;

	g_object_class_install_property (object_class, PROP_VALUE,
					 g_param_spec_boxed ("value",
							     NULL, NULL,
							     GCONF_TYPE_VALUE,
							     G_PARAM_READWRITE));

	gconf_cell_signals[CHANGED] =
		g_signal_new ("changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GConfCellRendererClass, changed),
			      (GSignalAccumulator) NULL, NULL,
			      gconf_marshal_VOID__STRING_BOXED,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING,
			      GCONF_TYPE_VALUE);
	gconf_cell_signals[CHECK_WRITABLE] =
		g_signal_new ("check_writable",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GConfCellRendererClass, changed),
			      (GSignalAccumulator) NULL, NULL,
			      gconf_marshal_BOOLEAN__STRING,
			      G_TYPE_BOOLEAN, 1,
			      G_TYPE_STRING);
}

static void
gconf_cell_renderer_init (GConfCellRenderer *renderer)
{
	g_object_set (renderer,
	              "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
	              NULL);
	
	renderer->text_renderer = gtk_cell_renderer_text_new ();
	g_object_ref_sink (renderer->text_renderer);

	renderer->toggle_renderer = gtk_cell_renderer_toggle_new ();
	g_object_ref_sink (renderer->toggle_renderer);
}



GtkCellRenderer *
gconf_cell_renderer_new (void)
{
  return GTK_CELL_RENDERER (g_object_new (GCONF_TYPE_CELL_RENDERER, NULL));
}

