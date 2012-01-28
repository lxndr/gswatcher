/* 
 * utils.h
 * 
 * Copyright (C) 2011-2012 GSTool Developer(s)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 */



#ifndef __GSQUERY__UTILS_H__
#define __GSQUERY__UTILS_H__

#include <glib.h>

G_BEGIN_DECLS


gchar *gsq_utf8_repair (const gchar *text);
void gsq_print_dump (const gchar *data, gsize length);


/* Float endianness conversion macros */

/* Taken from GStreamer code */

#ifndef GFLOAT_FROM_LE

inline static gfloat
GFLOAT_SWAP_LE_BE (gfloat in)
{
	union {
		guint32 i;
		gfloat f;
	} u;
	
	u.f = in;
	u.i = GUINT32_SWAP_LE_BE (u.i);
	return u.f;
}

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	#define GFLOAT_TO_LE(val)	((gfloat) (val))
	#define GFLOAT_TO_BE(val)	(GFLOAT_SWAP_LE_BE (val))
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	#define GFLOAT_TO_LE(val)	(GFLOAT_SWAP_LE_BE (val))
	#define GFLOAT_TO_BE(val)	((gfloat) (val))
#else
	#error unknow ENDIAN type
#endif

#define GFLOAT_FROM_LE(val)	(GFLOAT_TO_LE (val))
#define GFLOATFROM_BE(val)	(GFLOAT_TO_BE (val))

#endif


G_END_DECLS

#endif
