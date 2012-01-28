/* 
 * json.h
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



#ifndef __G_JSON_H__
#define __G_JSON_H__

#include <glib.h>

G_BEGIN_DECLS


typedef enum _JsonType JsonType;
enum _JsonType {
	JSON_TYPE_NULL,
	JSON_TYPE_STRING,
	JSON_TYPE_INTEGER,
	JSON_TYPE_FLOAT,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_ARRAY,
	JSON_TYPE_OBJECT
};


typedef struct _GJsonNode GJsonNode;


typedef struct _GJsonIter GJsonIter;
struct _GJsonIter {
	GJsonNode *node;
	union {
		guint index;
		GHashTableIter iter;
	};
};



GJsonNode *g_json_node_new ();
void g_json_node_free (GJsonNode *node);
GJsonNode *g_json_node_copy (GJsonNode *node);
JsonType g_json_node_get_type (GJsonNode *node);


GJsonNode *g_json_string_new (const gchar *value);
const gchar *g_json_string_get (GJsonNode *node);
void g_json_string_set (GJsonNode *node, const gchar *value);


GJsonNode *g_json_integer_new (gint64 value);
gint64 g_json_integer_get (GJsonNode *node);
void g_json_integer_set (GJsonNode *node, gint64 value);


GJsonNode *g_json_float_new (gdouble value);
gdouble g_json_float_get (GJsonNode *node);
void g_json_float_set (GJsonNode *node, gdouble value);


GJsonNode *g_json_boolean_new (gboolean value);
gboolean g_json_boolean_get (GJsonNode *node);
void g_json_boolean_set (GJsonNode *node, gboolean value);


GJsonNode *g_json_array_new ();
void g_json_array_add (GJsonNode *array, GJsonNode *node);
void g_json_arrat_remove (GJsonNode *array, GJsonNode *node);
GList *g_json_array_list (GJsonNode *array);


GJsonNode *g_json_object_new ();
gboolean g_json_object_has (GJsonNode *object, const gchar *name);
void g_json_object_set (GJsonNode *object, const gchar *name, GJsonNode *node);
#define g_json_object_set_string(obj, name, val) \
		g_json_object_set (obj, name, g_json_string_new (val))
#define g_json_object_set_boolean(obj, name, val) \
		g_json_object_set (obj, name, g_json_boolean_new (val))
#define g_json_object_set_integer(obj, name, val) \
		g_json_object_set (obj, name, g_json_integer_new (val))
#define g_json_object_set_float(obj, name, val) \
		g_json_object_set (obj, name, g_json_float_new (val))
GJsonNode *g_json_object_get (GJsonNode *object, const gchar *name);
#define g_json_object_get_string(obj, name) \
		g_json_string_get (g_json_object_get (obj, name))
#define g_json_object_get_boolean(obj, name) \
		g_json_boolean_get (g_json_object_get (obj, name))
#define g_json_object_get_integer(obj, name) \
		g_json_integer_get (g_json_object_get (obj, name))
#define g_json_object_get_float(obj, name) \
		g_json_float_get (g_json_object_get (obj, name))


//typedef gboolean (*JsonReader) (gchar *chr, gpointer udata);
//GJsonNode * g_json_read (JsonReader reader, gpointer udata, GError **error);
GJsonNode *g_json_read_from_string (const gchar *str, GError **error);
GJsonNode *g_json_read_from_file (const gchar *fname, GError **error);


typedef gboolean (*JsonWriter) (gchar *buffer, gsize size, gpointer udata);
void g_json_write (GJsonNode *node, JsonWriter writer, gpointer udata);
gchar *g_json_write_to_string (GJsonNode *node);
gboolean g_json_write_to_file (GJsonNode *node, const gchar *fname, GError **error);


void g_json_iter_init (GJsonIter *iter, GJsonNode *node);
gboolean g_json_iter_next_array (GJsonIter *iter, GJsonNode **child);
gboolean g_json_iter_next_object (GJsonIter *iter, gchar **name, GJsonNode **child);


G_END_DECLS

#endif