/*
 * Copyright © 2009 Christian Persch
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#if !defined (__TERMINAL_TERMINAL_H_INSIDE__) && !defined (TERMINAL_COMPILATION)
#error "Only <terminal/terminal.h> can be included directly."
#endif

#ifndef TERMINAL_VERSION_H
#define TERMINAL_VERSION_H

#define TERMINAL_MAJOR_VERSION (1)
#define TERMINAL_MINOR_VERSION (8)
#define TERMINAL_MICRO_VERSION (0)

#define TERMINAL_CHECK_VERSION(major,minor,micro) \
  (TERMINAL_MAJOR_VERSION > (major) || \
   (TERMINAL_MAJOR_VERSION == (major) && TERMINAL_MINOR_VERSION > (minor)) || \
   (TERMINAL_MAJOR_VERSION == (major) && TERMINAL_MINOR_VERSION == (minor) && TERMINAL_MICRO_VERSION >= (micro)))

#endif /* !TERMINAL_VERSION_H */
