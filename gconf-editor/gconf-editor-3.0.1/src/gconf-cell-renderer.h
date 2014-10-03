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

#ifndef __GCONF_CELL_RENDERER_H__
#define __GCONF_CELL_RENDERER_H__

#include <gtk/gtk.h>
#include <gconf/gconf-value.h>

#define GCONF_TYPE_CELL_RENDERER	    (gconf_cell_renderer_get_type ())
#define GCONF_CELL_RENDERER(obj)	    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCONF_TYPE_CELL_RENDERER, GConfCellRenderer))
#define GCONF_CELL_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GCONF_TYPE_CELL_RENDERER, GConfCellRendererClass))
#define GCONF_IS_CELL_RENDERER(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCONF_TYPE_CELL_RENDERER))
#define GCONF_IS_CELL_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_CELL_RENDERER))
#define GCONF_CELL_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCONF_TYPE_CELL_RENDERER, GConfCellRendererClass))

typedef struct _GConfCellRenderer GConfCellRenderer;
typedef struct _GConfCellRendererClass GConfCellRendererClass;

enum {
	GCONF_CELL_RENDERER_ICON_COLUMN,
	GCONF_CELL_RENDERER_KEY_NAME_COLUMN,
	GCONF_CELL_RENDERER_VALUE_COLUMN,
	GCONF_CELL_RENDERER_VALUE_TYPE_COLUMN,
	GCONF_CELL_RENDERER_NUM_COLUMNS
};

struct _GConfCellRenderer {
	GtkCellRenderer parent_instance;

	GConfValue *value;

	GtkCellRenderer *text_renderer;
	GtkCellRenderer *toggle_renderer;
};

struct _GConfCellRendererClass {
	GtkCellRendererClass parent_class;

	void (*changed) (GConfCellRenderer *renderer, char *path, GConfValue *value);
	gboolean (*check_writable) (GConfCellRenderer *renderer, char *path);
};

GType gconf_cell_renderer_get_type (void);
GtkCellRenderer *gconf_cell_renderer_new (void);

#endif /* __GCONF_CELL_RENDERER_H__ */
