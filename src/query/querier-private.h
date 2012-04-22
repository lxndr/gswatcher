/* 
 * querier-private.h: querier private functions
 * 
 * Copyright (C) 2011-2012 GSWatcher Developer(s)
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



#ifndef __GSQUERY__QUERIER_PRIVATE_H__
#define __GSQUERY__QUERIER_PRIVATE_H__

#include "querier.h"

G_BEGIN_DECLS


/* protocol private data */
void gsq_querier_set_pdata (GsqQuerier *querier, gpointer pdata);
gpointer gsq_querier_get_pdata (GsqQuerier *querier);

void gsq_querier_send (GsqQuerier *querier, guint16 port, const gchar *data,
		gsize length);
void gsq_querier_add_field (GsqQuerier *querier, const gchar *name, GType type);
void gsq_querier_add_player (GsqQuerier *querier, const gchar *name, ...);

void gsq_querier_set_extra (GsqQuerier *querier, const gchar *key,
		const gchar *value);

void gsq_querier_emit_info_update (GsqQuerier *querier);
void gsq_querier_emit_player_update (GsqQuerier *querier);
void gsq_querier_emit_log (GsqQuerier *querier, const gchar *msg);


G_END_DECLS

#endif
