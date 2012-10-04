/* 
 * proto-gamespy2.h: GameSpy 2 query protocol implementation
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



#ifndef __GSQUERY__PROTO_GAMESPY2_H__
#define __GSQUERY__PROTO_GAMESPY2_H__

#include "querier.h"

void gsq_gamespy2_query (GsqQuerier *querier);
gboolean gsq_gamespy2_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size);
void gsq_gamespy2_free (GsqQuerier *querier);

#endif
