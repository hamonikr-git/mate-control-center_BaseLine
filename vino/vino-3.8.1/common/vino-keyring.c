/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
 * Copyright (C) 2006 Jonh Wendell <wendell@bani.com.br>
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
 * Authors:
 *      Mark McLoughlin <mark@skynet.ie>
 *      Jonh Wendell <wendell@bani.com.br>
 *      Ryan Lortie <desrt@desrt.ca>
 */

#include "config.h"

#include "vino-keyring.h"

#ifdef VINO_ENABLE_KEYRING

/* TODO: canhas async? */

#include <secret/secret.h>

char *
vino_keyring_get_password (void)
{
  return secret_password_lookup (SECRET_SCHEMA_COMPAT_NETWORK,
                                 NULL, NULL,
                                 "server", "vino.local",
                                 "protocol", "rfb",
                                 "authtype", "vnc-password",
                                 "port", 5900,
                                 NULL);
}

gboolean
vino_keyring_set_password (const char *password)
{
  return secret_password_store_sync (SECRET_SCHEMA_COMPAT_NETWORK,
                                     SECRET_COLLECTION_DEFAULT,
                                     _("Remote desktop sharing password"),
                                     password, NULL, NULL,
                                     "server", "vino.local",
                                     "protocol", "rfb",
                                     "authtype", "vnc-password",
                                     "port", 5900,
                                     NULL);
}

#else

gchar *
vino_keyring_get_password (void)
{
  return NULL;
}

gboolean
vino_keyring_set_password (const gchar *password)
{
  return FALSE;
}

#endif
