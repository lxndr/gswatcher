/* 
 * console.c
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



#include <string.h>
#include <gio/gio.h>
#include "utils.h"
#include "console.h"


enum {
	PROP_0,
	PROP_ADDRESS,
	PROP_PASSWORD,
	PROP_TIMEOUT
};

enum {
	SIGNAL_CONNECTED,
	SIGNAL_DISCONNECTED,
	SIGNAL_AUTHENTICATED,
	LAST_SIGNAL
};


typedef struct _Request {
	gchar *command;
	GSimpleAsyncResult *result;
	gint attempts;
} Request;

struct _GsqConsolePrivate {
	gchar *address;
	gchar *password;
	GSocket *socket;
	guint source;
	guint timeout;
	guint timer;
	gboolean working;
	gboolean authenticated;
	GList *queue;
	guint chunk_size;
	GByteArray *chunk;
};


static guint signals[LAST_SIGNAL] = { 0 };


GQuark
gsq_console_error_quark (void)
{
	return g_quark_from_static_string ("gsq-console-error-quark");
}


static void establish_connection (GsqConsole *console);


G_DEFINE_TYPE (GsqConsole, gsq_console, G_TYPE_OBJECT);


static void
gsq_console_finalize (GObject *object)
{
	GsqConsole *console = GSQ_CONSOLE (object);
	GsqConsolePrivate *priv = console->priv;
	
	if (priv->address)
		g_free (priv->address);
	
	if (priv->password)
		g_free (priv->password);
	
	while (priv->queue) {
		Request *req = priv->queue->data;
		g_free (req->command);
		g_object_unref (req->result);
		priv->queue = g_list_delete_link (priv->queue, priv->queue);
	}
	
	if (priv->socket) {
		g_socket_close (priv->socket, NULL);
		g_object_unref (priv->socket);
	}
	
	if (priv->timer) {
		g_source_remove (priv->timer);
		priv->timer = 0;
	}
	
	g_byte_array_free (priv->chunk, TRUE);
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
	case PROP_TIMEOUT:
		gsq_console_set_timeout (console, g_value_get_uint (value));
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
	case PROP_TIMEOUT:
		g_value_set_uint (value, priv->timeout);
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
	
	g_object_class_install_property (gobject_class, PROP_TIMEOUT,
			g_param_spec_uint ("timeout", "Timeout", "Timeout",
			0, 120, 10, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));
	
	signals[SIGNAL_CONNECTED] = g_signal_new ("connected",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GsqConsoleClass, connected), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_DISCONNECTED] = g_signal_new ("disconnected",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GsqConsoleClass, disconnected), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_AUTHENTICATED] = g_signal_new ("authenticated",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, authenticated), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
gsq_console_init (GsqConsole *console)
{
	console->priv = G_TYPE_INSTANCE_GET_PRIVATE (console, GSQ_TYPE_CONSOLE,
			GsqConsolePrivate);
	console->priv->chunk = g_byte_array_new ();
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


void
gsq_console_set_timeout (GsqConsole *console, guint timeout)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	console->priv->timeout = timeout;
	g_object_notify (G_OBJECT (console), "timeout");
}

guint
gsq_console_get_timeout (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), 0);
	return console->priv->timeout;
}


static void
close_connection (GsqConsole *console)
{
	GsqConsoleClass *class = GSQ_CONSOLE_GET_CLASS (console);
	GsqConsolePrivate *priv = console->priv;
	priv->authenticated = FALSE;
	
	if (priv->timer) {
		g_source_remove (priv->timer);
		priv->timer = 0;
	}
	
	if (priv->socket) {
		if (priv->source) {
			g_source_remove (priv->source);
			priv->source = 0;
		}
		g_socket_close (priv->socket, NULL);
		g_object_unref (priv->socket);
		priv->socket = NULL;
	}
	
	g_byte_array_set_size (priv->chunk, 0);
	if (class->reset)
		class->reset (console);
	
	if (priv->queue)
		establish_connection (console);
}


static void
throw_error (GsqConsole *console, GError *error)
{
	GsqConsolePrivate *priv = console->priv;
	if (priv->queue) {
		Request *req = priv->queue->data;
		req->attempts--;
		if (req->attempts == 0) {
			priv->queue = g_list_delete_link (priv->queue, priv->queue);
			g_free (req->command);
			g_simple_async_result_take_error (req->result, error);
			g_simple_async_result_complete (req->result);
			g_object_unref (req->result);
			g_slice_free (Request, req);
		}
	}
	
	close_connection (console);
}


static gboolean
socket_timeout (GsqConsole *console)
{
	GError *error = NULL;
	g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_TIMEOUT, "Connection timed out");
	throw_error (console, error);
	return FALSE;
}


static void
start_timer (GsqConsole *console)
{
	g_return_if_fail (console->priv->timer == 0);
	
	GsqConsolePrivate *pr = console->priv;
	pr->timer = g_timeout_add_seconds (
			pr->timeout, (GSourceFunc) socket_timeout, console);
	pr->working = TRUE;
}

static void
stop_timer (GsqConsole *console)
{
	GsqConsolePrivate *pr = console->priv;
	g_source_remove (pr->timer);
	pr->timer = 0;
	pr->working = FALSE;
}


static void
send_request (GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	GsqConsoleClass *console_class = GSQ_CONSOLE_GET_CLASS (console);
	GError *error = NULL;
	
	if (priv->queue == NULL)
		return;
	
	Request *req = priv->queue->data;
	if (!console_class->command (console, req->command, &error)) {
		throw_error (console, error);
		return;
	}
	
	start_timer (console);
}


void
gsq_console_authenticate (GsqConsole *console)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	console->priv->authenticated = TRUE;
	g_signal_emit (console, signals[SIGNAL_AUTHENTICATED], 0);
	send_request (console);
}

gboolean
gsq_console_is_authenticated (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), FALSE);
	return console->priv->authenticated;
}


void
gsq_console_set_chunk_size (GsqConsole *console, guint size)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	console->priv->chunk_size = size;
}

guint
gsq_console_get_chunk_size (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), 0);
	return console->priv->chunk_size;
}


gboolean
gsq_console_send_data (GsqConsole *console, const gchar *data, gsize length,
		GError **error)
{
	GsqConsolePrivate *priv = console->priv;
	gssize ret;
	
	do {
		if ((ret = g_socket_send (priv->socket, data, length, NULL, error)) <= 0)
			return FALSE;
		data += ret;
		length -= ret;
	} while (length > 0);
	
	return TRUE;
}



static gboolean
socket_received (GSocket *socket, GIOCondition cond, GsqConsole *console)
{
	GsqConsolePrivate *pr = console->priv;
	GsqConsoleClass *console_class = GSQ_CONSOLE_GET_CLASS (console);
	GError *error = NULL;
	
	if (cond & G_IO_HUP) {
		g_set_error_literal (&error, GSQ_CONSOLE_ERROR, 0, "Socket has hung up");
		throw_error (console, error);
		return FALSE;
	}
	
	gchar data[4096];
	gssize length = pr->chunk_size > 0 ? pr->chunk_size - pr->chunk->len : 4096;
	length = g_socket_receive (socket, data, length, NULL, &error);
	if (length > 0) {
		if (pr->queue == NULL) {
			g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_DATA,
					"Odd data has been received");
			throw_error (console, error);
			return FALSE;
		}
		
		if (pr->chunk_size > 0) {
			/* gather data */
			g_byte_array_append (pr->chunk, (guint8 *) data, length);
			if (pr->chunk->len >= pr->chunk_size) {
				if (!console_class->received (console, (gchar *) pr->chunk->data,
						pr->chunk->len, &error)) {
					throw_error (console, error);
					return FALSE;
				}
				
				g_byte_array_set_size (pr->chunk, 0);
			}
		} else {
			/* if chunk_size == 0, do not accumulate data */
			if (console_class->received (console, data, length, &error)) {
				throw_error (console, error);
				return FALSE;
			}
		}
		return TRUE;
	} else if (length == 0) {
		close_connection (console);
		g_signal_emit (console, signals[SIGNAL_DISCONNECTED], 0);
	} else {
		throw_error (console, error);
	}
	
	return FALSE;
}


static gboolean
socket_connected (GSocket *socket, GIOCondition cond, GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	GError *error = NULL;
	
	if (g_socket_check_connect_result (socket, &error)) {
		if (priv->password && *priv->password) {
			stop_timer (console);
			g_socket_set_blocking (priv->socket, TRUE);
			g_signal_emit (console, signals[SIGNAL_CONNECTED], 0);
			
			/* create receivng source */
			GSource *source = g_socket_create_source (socket, G_IO_IN, NULL);
			g_source_set_callback (source, (GSourceFunc) socket_received,
					console, NULL);
			priv->source = g_source_attach (source, NULL);
		} else {
			g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_PASS,
					"Password is not specified");
			throw_error (console, error);
		}
	} else {
		throw_error (console, error);
	}
	
	return FALSE;
}

static void
address_resolved (GSocketAddressEnumerator *enumerator, GAsyncResult *result,
		GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	GError *error = NULL;
	GSocketAddress *saddr;
	
	g_return_if_fail (priv->socket == NULL);
	
	if (!(saddr = g_socket_address_enumerator_next_finish (enumerator, result,
			&error))) {
		throw_error (console, error);
		return;
	}
	
	/* create a socket */
	priv->socket = g_socket_new (g_socket_address_get_family (saddr),
			G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &error);
	if (!priv->socket) {
		throw_error (console, error);
		g_object_unref (saddr);
		return;
	}
	
	/* create connecting source */
	g_socket_set_blocking (priv->socket, FALSE);
	GSource *source = g_socket_create_source (priv->socket, G_IO_OUT, NULL);
	g_source_set_callback (source, (GSourceFunc) socket_connected, console, NULL);
	priv->source = g_source_attach (source, NULL);
	
	/* connect */
	g_socket_connect (priv->socket, saddr, NULL, &error);
	if (error) {
		if (error->code != G_IO_ERROR_PENDING)
			throw_error (console, error);
		g_error_free (error);
	}
	g_object_unref (saddr);
	
	start_timer (console);
}

static void
establish_connection (GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	
	GError *error = NULL;
	if (!(priv->password && *priv->password)) {
		g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_PASS,
				"Password is not specified");
		throw_error (console, error);
		return;
	}
	
	GSocketConnectable *connectable;
	GSocketAddressEnumerator *enumerator;
	
	if (!(connectable = g_network_address_parse (priv->address, 27015, &error))) {
		throw_error (console, error);
		return;
	}
	
	priv->working = TRUE;
	enumerator = g_socket_connectable_enumerate (connectable);
	g_socket_address_enumerator_next_async (enumerator, NULL,
			(GAsyncReadyCallback) address_resolved, console);
	g_object_unref (enumerator);
	g_object_unref (connectable);
}

void
gsq_console_send (GsqConsole *console, const gchar *command,
		GAsyncReadyCallback callback, gpointer udata)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	g_return_if_fail (command != NULL);
	gsq_console_send_full (console, command, 1, callback, udata);
}

void
gsq_console_send_full (GsqConsole *console, const gchar *command,
		gint attempts,
		GAsyncReadyCallback callback, gpointer udata)
{
	g_return_if_fail (GSQ_IS_CONSOLE (console));
	g_return_if_fail (command != NULL);
	g_return_if_fail (attempts > 0);
	GsqConsolePrivate *priv = console->priv;
	
	/* skip empty command */
	if (!*command)
		return;
	
	/* form a request */
	Request *req = g_slice_new0 (Request);
	req->command = g_strdup (command);
	req->result = g_simple_async_result_new (G_OBJECT (console), callback,
			udata, gsq_console_send);
	req->attempts = attempts;
	priv->queue = g_list_append (priv->queue, req);
	
	if (!priv->working) {
		if (priv->authenticated)
			send_request (console);
		else
			establish_connection (console);
	}
}

gchar *
gsq_console_send_finish (GsqConsole *console, GAsyncResult *result, GError **error)
{
	g_return_val_if_fail (g_simple_async_result_is_valid (result,
			G_OBJECT (console), gsq_console_send), NULL);
	
	GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
	if (g_simple_async_result_propagate_error (simple, error))
		return NULL;
	return g_simple_async_result_get_op_res_gpointer (simple);
}


guint
gsq_console_queue_length (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), 0);
	return g_list_length (console->priv->queue);
}


void
gsq_console_finish_respond (GsqConsole *console, const gchar *msg)
{
	GsqConsolePrivate *pr = console->priv;
	GsqConsoleClass *console_class = GSQ_CONSOLE_GET_CLASS (console);
	Request *req = (Request *) pr->queue->data;
	
	pr->queue = g_list_delete_link (pr->queue, pr->queue);
	stop_timer (console);
	
	g_free (req->command);
	gchar *output = gsq_utf8_repair (msg);
	g_simple_async_result_set_op_res_gpointer (req->result, output, g_free);
	g_simple_async_result_complete (req->result);
	g_object_unref (req->result);
	g_slice_free (Request, req);
	
	if (console_class->reset)
		console_class->reset (console);
	
	send_request (console);
}
