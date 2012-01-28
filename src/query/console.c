/* 
 * console.c: remote console implementation
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



#include <gio/gio.h>
#include "proto-source-console.h"
#include "console.h"


enum {
	PROP_0,
	PROP_ADDRESS,
	PROP_PASSWORD,
};

enum {
	SIGNAL_CONNECT,
	SIGNAL_DISCONNECT,
	SIGNAL_AUTHENTICATE,
	SIGNAL_RESPOND,
	SIGNAL_ERROR,
	LAST_SIGNAL
};

struct _GsqConsolePrivate {
	gchar *address;
	GSocket *socket;
	GSource *socket_source;
	GCancellable *cancellable;
	guint timer;
	gchar *password;
	gboolean connected;
};


static guint signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (GsqConsole, gsq_console, G_TYPE_OBJECT);


static void
gsq_console_finalize (GObject *object)
{
	GsqConsole *console = GSQ_CONSOLE (object);
	GsqConsolePrivate *priv = console->priv;
	
	gsq_console_disconnect (console);
	
	if (priv->address)
		g_free (priv->address);
	if (priv->password)
		g_free (priv->password);
	g_object_unref (console->priv->cancellable);
}


static void
gsq_console_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsqConsole *console = GSQ_CONSOLE (object);
	GsqConsolePrivate *priv = console->priv;
	
	switch (prop_id) {
	case PROP_ADDRESS:
		priv->address = g_strdup (g_value_get_string (value));
		break;
	case PROP_PASSWORD:
		gsq_console_set_password (console, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gsq_console_get_property (GObject *object, guint prop_id, GValue *value,
		GParamSpec *pspec)
{
	GsqConsole *console = GSQ_CONSOLE (object);
	GsqConsolePrivate *priv = console->priv;
	
	switch (prop_id) {
	case PROP_ADDRESS:
		g_value_set_string (value, priv->address);
		break;
	case PROP_PASSWORD:
		g_value_set_string (value, priv->password);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

static void
gsq_console_class_init (GsqConsoleClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->set_property = gsq_console_set_property;
	gobject_class->get_property = gsq_console_get_property;
	gobject_class->finalize = gsq_console_finalize;
	
	g_type_class_add_private (gobject_class, sizeof (GsqConsolePrivate));
	
	g_object_class_install_property (gobject_class, PROP_ADDRESS,
			g_param_spec_string ("address", "Address", "Address and port",
			NULL, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property (gobject_class, PROP_PASSWORD,
			g_param_spec_string ("password", "Password", "RCon password",
			NULL, G_PARAM_WRITABLE | G_PARAM_READABLE));
		
	signals[SIGNAL_CONNECT] = g_signal_new ("connect",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, connect), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_DISCONNECT] = g_signal_new ("disconnect",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, disconnect), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_RESPOND] = g_signal_new ("respond",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, respond), NULL, NULL,
			g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	
	signals[SIGNAL_ERROR] = g_signal_new ("error",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, error), NULL, NULL,
			g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
gsq_console_init (GsqConsole *console)
{
	console->priv = G_TYPE_INSTANCE_GET_PRIVATE (console, GSQ_TYPE_CONSOLE,
			GsqConsolePrivate);
	console->priv->cancellable = g_cancellable_new ();
}

GsqConsole *
gsq_console_new (const gchar *address)
{
	g_return_val_if_fail (address != NULL, NULL);
	return g_object_new (GSQ_TYPE_CONSOLE, "address", address, NULL);
}


void
gsq_console_set_password (GsqConsole *console, const gchar *password)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	if (console->priv->password)
		g_free (console->priv->password);
	console->priv->password = password ? g_strdup (password) : NULL;
	g_object_notify (G_OBJECT (console), "password");
}

const gchar *
gsq_console_get_password (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), NULL);
	return console->priv->password;
}


gboolean
g_socket_send_all (GSocket *socket, const gchar *buffer, gsize size,
		GCancellable *cancellable, GError **error)
{
	gssize ret;
	
	do {
		if ((ret = g_socket_send (socket, buffer, size, cancellable, error)) <= 0)
			return FALSE;
		buffer += ret;
		size -= ret;
	} while (size > 0);
	
	return TRUE;
}

gboolean
g_socket_receive_all (GSocket *socket, gchar *buffer, gsize size,
		GCancellable *cancellable, GError **error)
{
	gssize ret;
	
	do {
		if ((ret = g_socket_receive (socket, buffer, size, cancellable, error)) <= 0)
			return FALSE;
		buffer += ret;
		size -= ret;
	} while (size > 0);
	
	return TRUE;
}


static gboolean
gsq_console_recv (GSocket *socket, GIOCondition condition, GsqConsole *console)
{
	if (condition & G_IO_HUP) {
		g_signal_emit (console, signals[SIGNAL_ERROR], 0, "Connection unexpectedly hung up");
		gsq_console_disconnect (console);
		return FALSE;
	} else if (condition & G_IO_ERR) {
		g_signal_emit (console, signals[SIGNAL_ERROR], 0, "An error has occurred");
		gsq_console_disconnect (console);
		return FALSE;
	} else if (G_LIKELY (condition & G_IO_IN)) {
		GError *error = NULL;
		gboolean ret = gsq_source_console_process (console, console->priv->socket,
				console->priv->cancellable, &error);
		if (!ret) {
			if (error) {
				g_signal_emit (console, signals[SIGNAL_ERROR], 0, error->message);
				g_error_free (error);
			} else {
				gsq_console_disconnect (console);
			}
		}
		
		if (console->priv->timer) {
			g_source_remove (console->priv->timer);
			console->priv->timer = 0;
		}
		
		return ret;
	}
	
	return FALSE;
}

static gboolean
gsq_console_timeout (GsqConsole *console)
{
	console->priv->timer = 0;
	g_cancellable_cancel (console->priv->cancellable);
	g_signal_emit (console, signals[SIGNAL_ERROR], 0, "Connection was timed out");
	gsq_console_disconnect (console);
	return FALSE;
}


static void
gsq_console_connect_finish (GsqConsole *console, GAsyncResult *res, gpointer udata)
{
	GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (res);
	GError *error = NULL;
	if (g_simple_async_result_propagate_error (simple, &error)) {
		g_signal_emit (console, signals[SIGNAL_ERROR], 0, error->message);
		g_error_free (error);
		return;
	}
	
	console->priv->timer = g_timeout_add_seconds (
			4, (GSourceFunc) gsq_console_timeout, console);
	if (!gsq_source_console_init (console, console->priv->socket,
			console->priv->cancellable, &error)) {
		if (error) {
			g_signal_emit (console, signals[SIGNAL_ERROR], 0, error->message);
			g_error_free (error);
		} else {
			gsq_console_disconnect (console);
		}
	}
}


static void
gsq_console_connect_thread (GSimpleAsyncResult *result, GObject *object,
		GCancellable *cancellable)
{
	GsqConsole *console = GSQ_CONSOLE (object);
	GsqConsolePrivate *priv = console->priv;
	GError *error = NULL;
	
	/* parse */
	GSocketConnectable *connectable = g_network_address_parse (
			priv->address, 27015, &error);
	if (!connectable) {
#if GLIB_MINOR_VERSION >= 28
		g_simple_async_result_take_error (result, error);
#else
		g_simple_async_result_set_from_error (result, error);
		g_error_free (error);
#endif
		return;
	}
	
	/* resolve */
	GSocketAddressEnumerator *enumerator = g_socket_connectable_enumerate (connectable);
	GSocketAddress *socket_address = g_socket_address_enumerator_next (
			enumerator, cancellable, &error);
	g_object_unref (enumerator);
	g_object_unref (connectable);
	if (!socket_address) {
#if GLIB_MINOR_VERSION >= 28
		g_simple_async_result_take_error (result, error);
#else
		g_simple_async_result_set_from_error (result, error);
		g_error_free (error);
#endif
		return;
	}
	
	/* socket */
	priv->socket = g_socket_new (G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
			G_SOCKET_PROTOCOL_TCP, &error);
	if (!priv->socket) {
		g_object_unref (socket_address);
#if GLIB_MINOR_ERROR >= 28
		g_simple_async_result_take_error (result, error);
#else
		g_simple_async_result_set_from_error (result, error);
		g_error_free (error);
#endif
		return;
	}
	
	/* connect */
	if (!g_socket_connect (priv->socket, socket_address, cancellable, &error)) {
		g_object_unref (socket_address);
		g_object_unref (priv->socket);
		priv->socket = NULL;
#if GLIB_MINOR_ERROR >= 28
		g_simple_async_result_take_error (result, error);
#else
		g_simple_async_result_set_from_error (result, error);
		g_error_free (error);
#endif
		return;
	}
	
	/* source */
	priv->socket_source = g_socket_create_source (priv->socket, G_IO_IN, NULL);
	g_source_set_callback (priv->socket_source, (GSourceFunc) gsq_console_recv,
			console, NULL);
	g_source_attach (priv->socket_source, NULL);
	g_source_unref (priv->socket_source);
	
	g_object_unref (socket_address);
}

void
gsq_console_connect (GsqConsole *console)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	g_return_if_fail (console->priv->password != NULL);
	
	GSimpleAsyncResult *result = g_simple_async_result_new (G_OBJECT (console),
			(GAsyncReadyCallback) gsq_console_connect_finish, NULL,
			gsq_console_connect);
	g_simple_async_result_run_in_thread (result, gsq_console_connect_thread,
			G_PRIORITY_DEFAULT, console->priv->cancellable);
	g_object_unref (result);
}

void
gsq_console_set_connected (GsqConsole *console)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	console->priv->connected = TRUE;
	g_signal_emit (console, signals[SIGNAL_CONNECT], 0);
}

gboolean
gsq_console_is_connected (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), FALSE);
	return console->priv->connected;
}

void
gsq_console_disconnect (GsqConsole *console)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	
	/* stop timeout timer */
	if (console->priv->timer) {
		g_source_remove (console->priv->timer);
		console->priv->timer = 0;
	}
	
	/* remove socket source */
	if (console->priv->socket_source) {
		g_source_destroy (console->priv->socket_source);
		console->priv->socket_source = NULL;
	}
	
	/* close socket */
	if (console->priv->socket) {
		g_socket_close (console->priv->socket, NULL);
		g_object_unref (console->priv->socket);
		console->priv->socket = NULL;
		console->priv->connected = FALSE;
		g_signal_emit (console, signals[SIGNAL_DISCONNECT], 0);
	}
	
	/* set variables to their initial state */
	g_cancellable_reset (console->priv->cancellable);
}

GSocket *
gsq_console_get_socket (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), NULL);
	return console->priv->socket;
}


void
gsq_console_send (GsqConsole *console, const gchar *command)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	if (!console->priv->connected)
		return;
	
	GError *error = NULL;
	if (!gsq_source_console_send (console, console->priv->socket, command,
			console->priv->cancellable, &error)) {
		if (error) {
			g_signal_emit (console, signals[SIGNAL_ERROR], 0, error->message);
			g_error_free (error);
		} else {
			// disconnect
		}
	}
	
	if (console->priv->timer)
		g_source_remove (console->priv->timer);
	console->priv->timer = g_timeout_add_seconds (
			5, (GSourceFunc) gsq_console_timeout, console);
}
