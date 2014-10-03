/*
 * Copyright © 2010 Codethink Limited
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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include <glib.h>

typedef struct _VinoConnectivityInfo VinoConnectivityInfo;

VinoConnectivityInfo * vino_connectivity_info_new (gint screen_number);
gboolean vino_connectivity_info_get (VinoConnectivityInfo  *info,
                                     gchar                **internal_host,
                                     guint16               *internal_port,
                                     gchar                **external_host,
                                     guint16               *external_port,
                                     gchar                **avahi_host);
void vino_connectivity_info_check (VinoConnectivityInfo *info);
