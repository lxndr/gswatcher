/* 
 * console.c
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



#include <string.h>
#include <glib/gprintf.h>
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
	SIGNAL_CONNECT,
	SIGNAL_DISCONNECT,
	LAST_SIGNAL
};


typedef struct _Request {
	gchar *command;
	gchar key[14];	/* with this key we can found out if respond is complite */
	GString *output;
	GSimpleAsyncResult *result;
	gint attempts;
} Request;

typedef struct _Packet {
	gint32 size;
	gint32 id;
	gint32 type;
	gchar data[4096];
	gchar *p;
	gsize left;
} Packet;

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
	Packet packet;
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
		g_string_free (req->output, TRUE);
		g_object_unref (req->result);
		priv->queue = priv->queue->next;
	}
	
	if (priv->socket) {
		g_socket_close (priv->socket, NULL);
		g_object_unref (priv->socket);
	}
	if (priv->timer) {
		g_source_remove (priv->timer);
		priv->timer = 0;
	}
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
	
	signals[SIGNAL_CONNECT] = g_signal_new ("connect",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, connect), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_DISCONNECT] = g_signal_new ("disconnect",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqConsoleClass, disconnect), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
gsq_console_init (GsqConsole *console)
{
	console->priv = G_TYPE_INSTANCE_GET_PRIVATE (console, GSQ_TYPE_CONSOLE,
			GsqConsolePrivate);
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

gboolean
gsq_console_is_connected (GsqConsole *console)
{
	g_return_val_if_fail (GSQ_IS_CONSOLE (console), FALSE);
	return console->priv->authenticated;
}


static void
close_connection (GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	
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
	
	priv->working = FALSE;
	priv->authenticated = FALSE;
	priv->packet.size = 0;
	priv->packet.left = 0;
	
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
			g_string_free (req->output, TRUE);
			g_simple_async_result_take_error (req->result, error);
			g_simple_async_result_complete (req->result);
			g_object_unref (req->result);
			g_slice_free (Request, req);
		}
	}
	
	close_connection (console);
}


static gboolean
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

static gboolean
source_send_packet (GSocket *socket, gint32 id, gint32 type, const gchar *cmd,
		GCancellable *cancellable, GError **error)
{
	id = GINT32_TO_LE (id);
	type = GINT32_TO_LE (type);
	
	gsize len = strlen (cmd) + 1;
	gint32 size = GINT32_TO_LE (len + 9);
	
	return	g_socket_send_all (socket, (gchar *) &size, 4, cancellable, error) &&
			g_socket_send_all (socket, (gchar *) &id, 4, cancellable, error) &&
			g_socket_send_all (socket, (gchar *) &type, 4, cancellable, error) &&
			g_socket_send_all (socket, cmd, len, cancellable, error) &&
			g_socket_send_all (socket, "", 1, cancellable, error);
}


static gboolean
socket_timeout (GsqConsole *console)
{
	console->priv->timer = 0;
	GError *error = NULL;
	g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_TIMEOUT, "Connection timed out");
	throw_error (console, error);
	return FALSE;
}


#define source_send_password(sock, pass, cancel, error)	\
			source_send_packet (sock, 0, 3, pass, cancel, error)


static void
send_command (GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	if (priv->queue == NULL)
		return;
	Request *req = priv->queue->data;
	guint32 key = g_random_int ();
	g_sprintf (req->key, "END%X \n", key);
	gchar *cmd = g_strdup_printf ("%s;echo END%X", req->command, key);
	source_send_packet (priv->socket, 0, 2, cmd, NULL, NULL);
	g_free (cmd);
	
	priv->timer = g_timeout_add_seconds (priv->timeout, (GSourceFunc) socket_timeout, console);
	priv->working = TRUE;
}


static void
respond_received (GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	Request *req = (Request *) priv->queue->data;
	
	priv->queue = g_list_delete_link (priv->queue, priv->queue);
	
	g_free (req->command);
	gchar *output = gsq_utf8_repair (req->output->str);
	g_simple_async_result_set_op_res_gpointer (req->result, output, g_free);
	g_string_free (req->output, TRUE);
	g_simple_async_result_complete (req->result);
	g_object_unref (req->result);
	g_slice_free (Request, req);
	
	g_source_remove (priv->timer);
	priv->timer = 0;
	priv->working = FALSE;
	
	send_command (console);
}


static gboolean
socket_received (GSocket *socket, GIOCondition cond, GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	Packet *pkt = &priv->packet;
	GError *error = NULL;
	
	if (cond & G_IO_HUP) {
		g_set_error_literal (&error, GSQ_CONSOLE_ERROR, 0, "Socket has hung up");
		throw_error (console, error);
		return FALSE;
	}
	
	
	if (pkt->size == 0 && pkt->left == 0) {	/* new packet */
		pkt->left = 12;
		pkt->p = pkt->data;
	}
	
	gssize size = g_socket_receive (socket, pkt->p, pkt->left, NULL, &error);
	if (G_UNLIKELY (size == 0)) {			/* disconnect */
		close_connection (console);
		g_signal_emit (console, signals[SIGNAL_DISCONNECT], 0);
		return FALSE;
	} else if (G_UNLIKELY (size == -1)) {	/* error */
		throw_error (console, error);
		return FALSE;
	} else {
		if (priv->queue == NULL) {
			g_set_error_literal (&error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_DATA,
					"Odd data has been received");
			throw_error (console, error);
			return FALSE;
		}
		Request *req = (Request *) priv->queue->data;
		
		pkt->p += size;
		pkt->left -= size;
		if (pkt->left == 0) {
			if (pkt->size == 0) {			/* header */
				pkt->size = GINT32_FROM_LE (* (gint32 *) (pkt->data + 0));
				pkt->id = GINT32_FROM_LE (* (gint32 *) (pkt->data + 4));
				pkt->type = GINT32_FROM_LE (* (gint32 *) (pkt->data + 8));
				if (pkt->size < 10 || pkt->size > 4106 || !(pkt->type == 0 || pkt->type == 2)) {
					g_set_error_literal (&error, GSQ_CONSOLE_ERROR,
							GSQ_CONSOLE_ERROR_DATA,
							"Odd data has been received");
					throw_error (console, error);
					return FALSE;
				}
				pkt->p = pkt->data;
				pkt->left = pkt->size - 8;
			} else {						/* data */
				if (priv->packet.type == 2) {
					if (priv->packet.id == -1) {
						g_set_error_literal (&error, GSQ_CONSOLE_ERROR,
								GSQ_CONSOLE_ERROR_AUTH,
								"Authentication attempt failed");
						throw_error (console, error);
					} else {
						priv->working = FALSE;
						priv->authenticated = TRUE;
						g_source_remove (priv->timer);
						priv->timer = 0;
						g_signal_emit (console, signals[SIGNAL_CONNECT], 0);
						send_command (console);
					}
				} else {
					
					if (priv->authenticated) {
						gboolean final = FALSE;
						if (pkt->p - pkt->data >= 15 && strncmp (pkt->p - 15, req->key, 13) == 0) {
							*(pkt->p - 15) = 0;
							final = TRUE;
						}
						
						req->output = g_string_append (req->output, pkt->data);
						if (final)
							respond_received (console);
					}
					
				}
				/* clear everything for next packet */
				pkt->size = 0;
			}
		}
	}
	
	return TRUE;
}


static gboolean
socket_connected (GSocket *socket, GIOCondition cond, GsqConsole *console)
{
	GsqConsolePrivate *priv = console->priv;
	GError *error = NULL;
	
	if (g_socket_check_connect_result (socket, &error)) {
		if (priv->password && *priv->password) {
			g_source_remove (priv->timer);
			g_socket_set_blocking (priv->socket, TRUE);
			source_send_password (socket, priv->password, NULL, NULL);
			
			/* create receivng source */
			GSource *source = g_socket_create_source (socket, G_IO_IN, NULL);
			g_source_set_callback (source, (GSourceFunc) socket_received,
					console, NULL);
			priv->source = g_source_attach (source, NULL);
			
			priv->timer = g_timeout_add_seconds (priv->timeout,
					(GSourceFunc) socket_timeout, console);
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
	
	priv->timer = g_timeout_add_seconds (priv->timeout,
			(GSourceFunc) socket_timeout, console);
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
	req->output = g_string_sized_new (0);
	req->result = g_simple_async_result_new (G_OBJECT (console), callback,
			udata, gsq_console_send);
	req->attempts = attempts;
	priv->queue = g_list_append (priv->queue, req);
	
	if (!priv->working) {
		if (priv->authenticated)
			send_command (console);
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
