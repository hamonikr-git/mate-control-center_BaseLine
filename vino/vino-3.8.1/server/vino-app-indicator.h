/*
 * Copyright (C) 2010 Canonical Ltd
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
 *      Travis B. Hartwell <nafai@travishartwell.net>
 */

#ifndef __VINO_APP_INDICATOR_H__
#define __VINO_APP_INDICATOR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VINO_TYPE_APP_INDICATOR         (vino_app_indicator_get_type ())
#define VINO_APP_INDICATOR(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), VINO_TYPE_APP_INDICATOR, VinoAppIndicator))
#define VINO_APP_INDICATOR_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), VINO_TYPE_APP_INDICATOR, VinoAppIndicatorClass))
#define VINO_IS_APP_INDICATOR(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), VINO_TYPE_APP_INDICATOR))
#define VINO_IS_APP_INDICATOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), VINO_TYPE_APP_INDICATOR))
#define VINO_APP_INDICATOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), VINO_TYPE_APP_INDICATOR, VinoAppIndicatorClass))

typedef struct _VinoAppIndicator        VinoAppIndicator;
typedef struct _VinoAppIndicatorClass   VinoAppIndicatorClass;
typedef struct _VinoAppIndicatorPrivate VinoAppIndicatorPrivate;

struct _VinoAppIndicator
{
  GObject          base;

  VinoAppIndicatorPrivate *priv;
};

struct _VinoAppIndicatorClass
{
  GObjectClass base_class;
};

#include "vino-server.h"
#include "vino-status-icon.h"

GType           vino_app_indicator_get_type      (void) G_GNUC_CONST;

VinoAppIndicator *vino_app_indicator_new           (VinoServer      *server);

VinoServer     *vino_app_indicator_get_server    (VinoAppIndicator  *indicator);

void            vino_app_indicator_add_client    (VinoAppIndicator  *indicator,
						  VinoClient      *client);
gboolean        vino_app_indicator_remove_client (VinoAppIndicator  *indicator,
						  VinoClient      *client);

void		vino_app_indicator_update_state	(VinoAppIndicator *indicator);

void		vino_app_indicator_set_visibility	(VinoAppIndicator           *indicator,
							 VinoStatusIconVisibility visibility);
VinoStatusIconVisibility vino_app_indicator_get_visibility (VinoAppIndicator *indicator);

G_END_DECLS

#endif /* __VINO_APP_INDICATOR_H__ */
