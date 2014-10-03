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

#include "vino-radio-button.h"

#include <gtk/gtk.h>

typedef struct
{
  GtkRadioButton parent_instance;
  gchar *name;
} VinoRadioButton;

typedef GtkRadioButtonClass VinoRadioButtonClass;

G_DEFINE_TYPE (VinoRadioButton, vino_radio_button, GTK_TYPE_RADIO_BUTTON)

enum
{
  PROP_0,
  PROP_SETTINGS_NAME,
  PROP_SETTINGS_ACTIVE
};

static void
vino_radio_button_get_property (GObject *object, guint prop_id,
                                GValue *value, GParamSpec *pspec)
{
  VinoRadioButton *vrb = (VinoRadioButton *) object;

  switch (prop_id)
    {
    case PROP_SETTINGS_ACTIVE:
      {
        const GSList *list;

        list = gtk_radio_button_get_group (&vrb->parent_instance);
        while (list)
          {
            VinoRadioButton *this = list->data;

            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (this)))
              {
                g_value_set_string (value, this->name);
                return;
              }

            list = list->next;
          }
      }

      g_warning ("No active radio buttons");
      g_value_set_string (value, "");
      return;

    default:
      g_assert_not_reached ();
    }
}

static void
vino_radio_button_set_property (GObject *object, guint prop_id,
                                const GValue *value, GParamSpec *pspec)
{
  VinoRadioButton *vrb = (VinoRadioButton *) object;

  switch (prop_id)
    {
    case PROP_SETTINGS_NAME:
      g_assert (vrb->name == NULL);
      vrb->name = g_value_dup_string (value);
      return;

    case PROP_SETTINGS_ACTIVE:
      {
        const GSList *list;
        const gchar *name;

        list = gtk_radio_button_get_group (&vrb->parent_instance);
        name = g_value_get_string (value);

        while (list)
          {
            VinoRadioButton *this = list->data;

            if (g_strcmp0 (this->name, name))
              {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this),
                                              TRUE);
                return;
              }

            list = list->next;
          }

        g_warning ("No such radio button named `%s'", name);
      }
      return;

    default:
      g_assert_not_reached ();
    }
}

static void
vino_radio_button_toggled (GtkToggleButton *button)
{
  VinoRadioButton *vrb = (VinoRadioButton *) button;

  /* As it is, we get the notification of the old button becoming inactivity
   * followed by the notification of the new button becoming active.  Only run
   * when the new button becomes active in order to avoid unnecessary
   * notifications.
   */
  if (gtk_toggle_button_get_active (button))
    {
      const GSList *list;

      list = gtk_radio_button_get_group (&vrb->parent_instance);
      while (list)
        {
          g_object_notify (list->data, "settings-active");
          list = list->next;
        }
    }
}

static void
vino_radio_button_finalize (GObject *object)
{
  VinoRadioButton *vrb = (VinoRadioButton *) object;

  g_free (vrb->name);

  G_OBJECT_CLASS (vino_radio_button_parent_class)
    ->finalize (object);
}

static void
vino_radio_button_init (VinoRadioButton *button)
{
}

static void
vino_radio_button_class_init (GtkRadioButtonClass *class)
{
  GtkToggleButtonClass *tb_class = GTK_TOGGLE_BUTTON_CLASS (class);
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  tb_class->toggled = vino_radio_button_toggled;
  object_class->get_property = vino_radio_button_get_property;
  object_class->set_property = vino_radio_button_set_property;
  object_class->finalize = vino_radio_button_finalize;

  g_object_class_install_property (object_class, PROP_SETTINGS_NAME,
    g_param_spec_string ("settings-name", "name", "name", NULL,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_SETTINGS_ACTIVE,
    g_param_spec_string ("settings-active", "active", "active", NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
