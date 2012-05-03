/* 
 * console-telnet.h: remote console via telnet protocol
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



#ifndef __GSQUERY__CONSOLE_TELNET_H__
#define __GSQUERY__CONSOLE_TELNET_H__

#include "console.h"


G_BEGIN_DECLS


#define GSQ_TYPE_CONSOLE_TELNET				(gsq_console_telnet_get_type ())
#define GSQ_CONSOLE_TELNET(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_CONSOLE_TELNET, GsqConsoleTelnet))
#define GSQ_IS_CONSOLE_TELNET(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_CONSOLE_TELNET))
#define GSQ_CONSOLE_TELNET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_CONSOLE_TELNET, GsqConsoleTelnetClass))
#define GSQ_IS_CONSOLE_TELNET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_CONSOLE_TELNET))
#define GSQ_CONSOLE_TELNET_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_CONSOLE_TELNET, GsqConsoleTelnetClass))

typedef struct _GsqConsoleTelnet			GsqConsoleTelnet;
typedef struct _GsqConsoleTelnetClass		GsqConsoleTelnetClass;

struct _GsqConsoleTelnet {
	GsqConsole parent_instance;
};

struct _GsqConsoleTelnetClass {
	GsqConsoleClass parent_class;
};

GType gsq_console_telnet_get_type (void) G_GNUC_CONST;
GsqConsole* gsq_console_telnet_new (const gchar *address);


G_END_DECLS

#endif
