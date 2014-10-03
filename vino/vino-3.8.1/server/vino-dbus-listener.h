/*
 * Copyright (C) 2004-2006 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2006 Jonh Wendell <wendell@bani.com.br>
 * Copyright (C) 2007 Mark McLoughlin <markmc@skynet.ie>
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
 *      William Jon McCann <mccann@jhu.edu>
 *      Jonh Wendell <wendell@bani.com.br>
 *      Mark McLoughlin <mark@skynet.ie>
 *
 * Code taken from gnome-screensaver/src/gs-listener-dbus.h
 */

#ifndef __VINO_DBUS_LISTENER_H__
#define __VINO_DBUS_LISTENER_H__

#include "vino-server.h"

typedef struct _VinoDBusListener        VinoDBusListener;

VinoDBusListener *      vino_dbus_listener_new          (gint screen);
void                    vino_dbus_listener_set_server   (VinoDBusListener *listener,
                                                         VinoServer       *server);
VinoServer *            vino_dbus_listener_get_server   (VinoDBusListener *self);

gboolean        vino_dbus_request_name     (void);

#endif /* __VINO_DBUS_LISTENER_H__ */
