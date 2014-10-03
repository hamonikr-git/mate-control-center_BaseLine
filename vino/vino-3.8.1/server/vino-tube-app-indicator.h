/*
 * © 2010, Canonical Ltd
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

#ifndef __VINO_TUBE_APP_INDICATOR_H__
#define __VINO_TUBE_APP_INDICATOR_H__

#include <gdk/gdk.h>

#include "vino-tube-server.h"

G_BEGIN_DECLS

#define VINO_TYPE_TUBE_APP_INDICATOR (vino_tube_app_indicator_get_type ())
#define VINO_TUBE_APP_INDICATOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
    VINO_TYPE_TUBE_APP_INDICATOR, VinoTubeAppIndicator))
#define VINO_TUBE_APP_INDICATOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), \
    VINO_TYPE_TUBE_APP_INDICATOR, VinoTubeAppIndicatorClass))
#define VINO_IS_TUBE_APP_INDICATOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
    VINO_TYPE_TUBE_APP_INDICATOR))
#define VINO_IS_TUBE_APP_INDICATOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), \
    VINO_TYPE_TUBE_APP_INDICATOR))
#define VINO_TUBE_APP_INDICATOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), \
    VINO_TYPE_TUBE_APP_INDICATOR, VinoTubeAppIndicatorClass))

typedef struct _VinoTubeAppIndicator VinoTubeAppIndicator;
typedef struct _VinoTubeAppIndicatorClass VinoTubeAppIndicatorClass;
typedef struct _VinoTubeAppIndicatorPrivate VinoTubeAppIndicatorPrivate;

struct _VinoTubeAppIndicator
{
  GObject base;
  VinoTubeAppIndicatorPrivate *priv;
};

struct _VinoTubeAppIndicatorClass
{
  GObjectClass base_class;
};

GType vino_tube_app_indicator_get_type (void) G_GNUC_CONST;

VinoTubeAppIndicator* vino_tube_app_indicator_new (VinoTubeServer *server);

void vino_tube_app_indicator_update_state (VinoTubeAppIndicator *indicator);

void vino_tube_app_indicator_set_visibility (VinoTubeAppIndicator *indicator,
    guint visibility);

void vino_tube_app_indicator_show_notif (VinoTubeAppIndicator *indicator,
    const gchar *summary, const gchar *body, gboolean invalidated);
G_END_DECLS

#endif /* __VINO_TUBE_APP_INDICATOR_H__ */
