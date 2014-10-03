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

#ifndef __GCONF_BOOKMARKS_H__
#define __GCONF_BOOKMARKS_H__

#include "gconf-editor-window.h"
#include <gtk/gtk.h>

void gconf_bookmarks_hook_up_menu (GConfEditorWindow *window,
				   GtkWidget         *menu,
				   GtkWidget         *add_bookmark,
				   GtkWidget         *edit_bookmarks);
void gconf_bookmarks_add_bookmark (const char *path);

#endif /* __GCONF_BOOKMARKS_H__ */
