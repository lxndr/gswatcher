/* 
 * json.c
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
#include "json.h"


static gchar *g_json_escape_string (const gchar *str);


struct _GJsonNode {
	JsonType type;
	union {
		gchar *s;
		gint64 i;
		gdouble f;
		gboolean b;
		GPtrArray *a;
		GHashTable *o;
	};
};


GJsonNode *
g_json_node_new ()
{
	return g_slice_new0 (GJsonNode);
}


void
g_json_node_free (GJsonNode *node)
{
	switch (node->type) {
	case JSON_TYPE_STRING:
		g_free (node->s);
		break;
	case JSON_TYPE_ARRAY:
		g_ptr_array_free (node->a, TRUE);
		break;
	case JSON_TYPE_OBJECT:
		g_hash_table_destroy (node->o);
		break;
	default:
		break;
	}
	
	g_slice_free (GJsonNode, node);
}


static void
g_json_node_reset (GJsonNode *node, JsonType type)
{
	switch (node->type) {
	case JSON_TYPE_STRING:
		g_free (node->s);
		break;
	case JSON_TYPE_ARRAY:
		g_ptr_array_free (node->a, TRUE);
		break;
	case JSON_TYPE_OBJECT:
		g_hash_table_destroy (node->o);
		break;
	default:
		break;
	}
	
	node->type = type;
	
	switch (type) {
	case JSON_TYPE_ARRAY:
		node->a = g_ptr_array_new_with_free_func ((GDestroyNotify) g_json_node_free);
		break;
	case JSON_TYPE_OBJECT:
		node->o = g_hash_table_new_full (g_str_hash, g_str_equal,
				(GDestroyNotify) g_free, (GDestroyNotify) g_json_node_free);
		break;
	default:
		break;
	}
}


static void
g_json_node_copy_array (GJsonNode *node, GJsonNode *array)
{
	g_json_array_add (array, g_json_node_copy (node));
}

static void
g_json_node_copy_object (gchar *name, GJsonNode *node, GJsonNode *object)
{
	g_json_object_set (object, name, g_json_node_copy (node));
}

GJsonNode *
g_json_node_copy (GJsonNode *node)
{
	switch (node->type) {
	case JSON_TYPE_NULL:
		return g_json_node_new ();
	case JSON_TYPE_STRING:
		return g_json_string_new (node->s);
/*	case JSON_TYPE_NUMBER:
		json_escape_number (json_number_get (node), buffer, maxsize);
		break;*/
	case JSON_TYPE_BOOLEAN:
		return g_json_boolean_new (node->b);
	case JSON_TYPE_ARRAY: {
			GJsonNode *array = g_json_node_new ();
			g_ptr_array_foreach (node->a, (GFunc) g_json_node_copy_array, array);
			return array;
		}
	case JSON_TYPE_OBJECT: {
			GJsonNode *object = g_json_node_new ();
			g_hash_table_foreach (node->o, (GHFunc) g_json_node_copy_object, object);
			return object;
		}
	default:
		return NULL;
	}
}


JsonType
g_json_node_get_type (GJsonNode *node)
{
	return node->type;
}



/* Reader */

typedef enum _GJsonToken {
	G_JSON_TOKEN_EOS,
	G_JSON_TOKEN_NULL,
	G_JSON_TOKEN_TRUE,
	G_JSON_TOKEN_FALSE,
	G_JSON_TOKEN_STRING,
	G_JSON_TOKEN_INTEGER,
	G_JSON_TOKEN_FLOAT,
	G_JSON_TOKEN_VALUE_SEPARATOR,
	G_JSON_TOKEN_NAME_SEPARATOR,
	G_JSON_TOKEN_ARRAY_START,
	G_JSON_TOKEN_ARRAY_END,
	G_JSON_TOKEN_OBJECT_START,
	G_JSON_TOKEN_OBJECT_END,
	G_JSON_TOKEN_UNKNOWN
} GJsonToken;

typedef struct _GJsonLexer {
	gchar *data, *p;
	gint line;
	
	GJsonToken type;
	GString *s;
	gint64 i;
	gdouble f;
} GJsonLexer;

static gchar *g_json_unescape_string (const gchar *str);

static void
g_json_lexer_next (GJsonLexer *lexer)
{
	for (;;) {
		if (*lexer->p == '\t' || *lexer->p == ' ') {
			lexer->p++;
		} else if (*lexer->p == '\n') {
			lexer->p++;
			lexer->line++;
		} else if (*lexer->p == '\r') {
			lexer->p++;
			if (*lexer->p == '\n')
				lexer->p++;
			lexer->line++;
		} else {
			break;
		}
	}
	
	switch (*lexer->p) {
	case ',':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_VALUE_SEPARATOR;
		break;
	case ':':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_NAME_SEPARATOR;
		break;
	case '[':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_ARRAY_START;
		break;
	case ']':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_ARRAY_END;
		break;
	case '{':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_OBJECT_START;
		break;
	case '}':
		lexer->p++;
		lexer->type = G_JSON_TOKEN_OBJECT_END;
		break;
	case '"':
		g_string_truncate (lexer->s, 0);
		lexer->p++;
		while (*lexer->p != '"') {
			if (*lexer->p == '\\') {
//				lexer->p++;
				if (*lexer->p++ == '\0') {
					lexer->type = G_JSON_TOKEN_EOS;
					return;
				}
				g_string_append_c (lexer->s, '\\');
			} else if (*lexer->p == '\0') {
				lexer->type = G_JSON_TOKEN_EOS;
				return;
			}
			
			g_string_append_c (lexer->s, *lexer->p++);
//			lexer->p++;
		}
		lexer->p++;
		lexer->type = G_JSON_TOKEN_STRING;
		break;
	case '\0':
		lexer->type = G_JSON_TOKEN_EOS;
		break;
	default:
		g_string_erase (lexer->s, 0, -1);
		while ((*lexer->p >= 'a' && *lexer->p <= 'z') ||
				(*lexer->p >= '0' && *lexer->p <= '9') ||
				*lexer->p == '-' || *lexer->p == '.' || *lexer->p == '_') {
			g_string_append_c (lexer->s, *lexer->p);
			lexer->p++;
		}
		
		if ((*lexer->s->str >= '0' && *lexer->s->str <= '9') ||
				*lexer->s->str == '.' || *lexer->s->str == '-') {
			if (strchr (lexer->s->str, '.')) {
				lexer->f = g_ascii_strtod (lexer->s->str, NULL);
				lexer->type = G_JSON_TOKEN_FLOAT;
			} else {
				lexer->i = g_ascii_strtoll (lexer->s->str, NULL, 10);
				lexer->type = G_JSON_TOKEN_INTEGER;
			}
		} else {
			if (strcmp (lexer->s->str, "null") == 0)
				lexer->type = G_JSON_TOKEN_NULL;
			else if (strcmp (lexer->s->str, "true") == 0)
				lexer->type = G_JSON_TOKEN_TRUE;
			else if (strcmp (lexer->s->str, "false") == 0)
				lexer->type = G_JSON_TOKEN_FALSE;
			else
				lexer->type = G_JSON_TOKEN_UNKNOWN;		
		}
	}
}

static GJsonLexer *
g_json_lexer_new (const gchar *data, gsize length)
{
	GJsonLexer *lexer = g_slice_new0 (GJsonLexer);
	if (length == -1)
		length = strlen (data);
	lexer->p = lexer->data = g_malloc (length + 1);
	strncpy (lexer->data, data, length);
	lexer->data[length] = 0;
	lexer->line = 1;
	lexer->s = g_string_sized_new (128);
	g_json_lexer_next (lexer);
	return lexer;
}

static void
g_json_lexer_free (GJsonLexer *lexer)
{
	g_free (lexer->data);
	g_string_free (lexer->s, TRUE);
	g_slice_free (GJsonLexer, lexer);
}


static GJsonNode *g_json_read_node (GJsonLexer *lexer, GError **error);

static GJsonNode *
g_json_read_array (GJsonLexer *lexer, GError **error)
{
	gboolean start = TRUE;
	GJsonNode *node = g_json_array_new ();
	
	for (;;) {
		if (lexer->type == G_JSON_TOKEN_ARRAY_END) {
			return node;
		} else if (start || lexer->type == G_JSON_TOKEN_VALUE_SEPARATOR) {
			if (!start)
				g_json_lexer_next (lexer);
			GJsonNode *child = g_json_read_node (lexer, error);
			if (!child)
				break;
			g_json_array_add (node, child);
			start = FALSE;
		} else if (lexer->type == G_JSON_TOKEN_EOS) {
			g_set_error (error, 0, 0, "unexpected end of stream");
			break;
		} else {
			g_set_error (error, 0, 0, "unexpected token '%s' on line %d",
					lexer->s->str, lexer->line);
			break;
		}
	}
	
	g_json_node_free (node);
	return NULL;
}

static GJsonNode *
g_json_read_object (GJsonLexer *lexer, GError **error)
{
	gboolean start = TRUE;
	GJsonNode *node = g_json_object_new ();
	
	for (;;) {
		if (lexer->type == G_JSON_TOKEN_OBJECT_END) {
			return node;
		} else if (start || lexer->type == G_JSON_TOKEN_VALUE_SEPARATOR) {
			if (!start)
				g_json_lexer_next (lexer);
			if (lexer->type != G_JSON_TOKEN_STRING) {
				g_set_error (error, 0, 0, "expected a string token on line %d",
						lexer->line);
				break;
			}
			gchar *name = g_json_unescape_string (lexer->s->str);
			g_json_lexer_next (lexer);
			if (lexer->type != G_JSON_TOKEN_NAME_SEPARATOR) {
				g_set_error (error, 0, 0, "expected a name token on line %d",
						lexer->line);
				break;
			}
			g_json_lexer_next (lexer);
			GJsonNode *child = g_json_read_node (lexer, error);
			if (!child)
				break;
			g_json_object_set (node, name, child);
			g_free (name);
			start = FALSE;
		} else if (lexer->type == G_JSON_TOKEN_EOS) {
			g_set_error (error, 0, 0, "unexpected end of stream");
			break;
		} else {
			g_set_error (error, 0, 0, "unexpected token '%s' on line %d",
					lexer->s->str, lexer->line);
			break;
		}
	}
	
	g_json_node_free (node);
	return NULL;
}

GJsonNode *
g_json_read_node (GJsonLexer *lexer, GError **error)
{
	GJsonNode *node;
	
	switch (lexer->type) {
	case G_JSON_TOKEN_NULL:
		node = g_json_node_new ();
		break;
	case G_JSON_TOKEN_TRUE:
		node = g_json_boolean_new (TRUE);
		break;
	case G_JSON_TOKEN_FALSE:
		node = g_json_boolean_new (FALSE);
		break;
	case G_JSON_TOKEN_STRING: {
		gchar *str = g_json_unescape_string (lexer->s->str);
		node = g_json_string_new (str);
		g_free (str);
		} break;
	case G_JSON_TOKEN_INTEGER:
		node = g_json_integer_new (lexer->i);
		break;
	case G_JSON_TOKEN_FLOAT:
		node = g_json_float_new (lexer->f);
		break;
	case G_JSON_TOKEN_ARRAY_START:
		g_json_lexer_next (lexer);
		node = g_json_read_array (lexer, error);
		break;
	case G_JSON_TOKEN_OBJECT_START:
		g_json_lexer_next (lexer);
		node = g_json_read_object (lexer, error);
		break;
	case G_JSON_TOKEN_EOS:
		return NULL;
	default:
		g_set_error (error, 0, 0, "unexpected token on line %d", lexer->line);
		return NULL;
	}
	
	g_json_lexer_next (lexer);
	return node;
}


GJsonNode *
g_json_read_from_string (const gchar *str, GError **error)
{
	GJsonLexer *lexer = g_json_lexer_new (str, -1);
	GJsonNode *node = g_json_read_node (lexer, error);
	g_json_lexer_free (lexer);
	return node;
}

GJsonNode *g_json_read_from_file (const gchar *fname, GError **error)
{
	gchar *data;
	gsize length;
	if (!g_file_get_contents (fname, &data, &length, error))
		return NULL;
	GJsonLexer *lexer = g_json_lexer_new (data, length);
	g_free (data);
	GJsonNode *node = g_json_read_node (lexer, error);
	g_json_lexer_free (lexer);
	return node;
}



/* Writer */

static void
g_json_node_write_spaces (JsonWriter writer, gpointer udata, gint space)
{
	while (space--)
		writer ("\t", 1, udata);
}

static void
g_json_node_write (GJsonNode *node, JsonWriter writer, gpointer udata, gint space)
{
	switch (node->type) {
	case JSON_TYPE_NULL:
		writer ("null", 4, udata);
		break;
	case JSON_TYPE_STRING: {
		writer ("\"", 1, udata);
		gchar *escaped = g_json_escape_string (node->s);
		writer (escaped, strlen (escaped), udata);
		g_free (escaped);
		writer ("\"", 1, udata);
	} break;
	case JSON_TYPE_INTEGER: {
		gchar buffer[64];
		gint length = g_sprintf (buffer, "%"G_GINT64_FORMAT, node->i);
		writer (buffer, length, udata);
		} break;
	case JSON_TYPE_FLOAT: {
		gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, node->f);
		writer (buffer, strlen (buffer), udata);
		} break;
	case JSON_TYPE_BOOLEAN:
		writer (node->b ? "true" : "false", node->b ? 4 : 5, udata);
		break;
	case JSON_TYPE_ARRAY: {
		GJsonIter iter;
		GJsonNode *child;
		gboolean first = TRUE;
		
		writer ("[", 1, udata);
		g_json_iter_init (&iter, node);
		while (g_json_iter_next_array (&iter, &child)) {
			if (first) {
				writer ("\n", 1, udata);
				first = FALSE;
			} else {
				writer (",\n", 2, udata);
			}
			g_json_node_write_spaces (writer, udata, space);
			g_json_node_write (child, writer, udata, space + 1);
		}
		writer ("\n", 1, udata);
		g_json_node_write_spaces (writer, udata, space - 1);
		writer ("]", 1, udata);
	} break;
	case JSON_TYPE_OBJECT: {
		GJsonIter iter;
		gchar *name;
		GJsonNode *child;
		gboolean first = TRUE;
		
		writer ("{", 1, udata);
		g_json_iter_init (&iter, node);
		while (g_json_iter_next_object (&iter, &name, &child)) {
			if (first) {
				writer ("\n", 1, udata);
				first = FALSE;
			} else {
				writer (",\n", 2, udata);
			}
			
			g_json_node_write_spaces (writer, udata, space);
			writer ("\"", 1, udata);
			
			gchar *escaped = g_json_escape_string (name);
			writer (escaped, strlen (escaped), udata);
			g_free (escaped);
			
			writer ("\": ", 3, udata);
			g_json_node_write (child, writer, udata, space + 1);
		}
		writer ("\n", 1, udata);
		g_json_node_write_spaces (writer, udata, space - 1);
		writer ("}", 1, udata);
	} break;
	default:
		break;
	}
}

void
g_json_write (GJsonNode *node, JsonWriter writer, gpointer udata)
{
	g_json_node_write (node, writer, udata, 1);
}


static gboolean
g_json_write_to_string_writer (gchar *buffer, gsize size, GString **string)
{
	*string = g_string_append_len (*string, buffer, size);
	return TRUE;
}

gchar *
g_json_write_to_string (GJsonNode *node)
{
	GString *string = g_string_sized_new (128);
	g_json_write (node, (JsonWriter) g_json_write_to_string_writer, (gpointer) &string);
	gchar *str = string->str;
	g_string_free (string, FALSE);
	return str;
}

static gboolean
g_json_write_to_file_writer (gchar *buffer, gsize size, GOutputStream *stream)
{
	return g_output_stream_write (stream, buffer, size, NULL, NULL);
}

gboolean
g_json_write_to_file (GJsonNode *node, const gchar *fname, GError **error)
{
	GFile *file = g_file_new_for_path (fname);
	GFileOutputStream *stream = g_file_replace (file, NULL, FALSE,
			G_FILE_CREATE_NONE, NULL, error);
	if (!stream) {
		g_object_unref (stream);
		g_object_unref (file);
		return FALSE;
	}
	
	g_json_write (node, (JsonWriter) g_json_write_to_file_writer, (gpointer) stream);
	
	g_object_unref (stream);
	g_object_unref (file);
	return TRUE;
}


/* String */

static gboolean
g_json_escape_string_eval (const GMatchInfo *info, GString *result, gpointer udata)
{
	gchar *match;
	
	match = g_match_info_fetch (info, 0);
	switch (match[0]) {
		case '\"':	g_string_append (result, "\\\"");	break;
		case '\\':	g_string_append (result, "\\\\");	break;
		case '/':	g_string_append (result, "\\/");	break;
		case '\b':	g_string_append (result, "\\b");	break;
		case '\f':	g_string_append (result, "\\f");	break;
		case '\n':	g_string_append (result, "\\n");	break;
		case '\r':	g_string_append (result, "\\r");	break;
		case '\t':	g_string_append (result, "\\t");	break;
	}
	g_free (match);
	
	return FALSE;
}

static gchar *
g_json_escape_string (const gchar *str)
{
	GRegex *regex = g_regex_new ("[\"\\\\/\\x08\\f\\n\\r\\t]", 0, 0, NULL);
	gchar *result = g_regex_replace_eval (regex, str, -1, 0, 0,
			g_json_escape_string_eval, NULL, NULL);
	g_regex_unref (regex);
	return result;
}


static gboolean
g_json_unescape_string_eval (const GMatchInfo *info, GString *result, gpointer udata)
{
	gchar *match;
	
	match = g_match_info_fetch (info, 0);
	switch (match[1]) {
		case '\"':	g_string_append (result, "\"");	break;
		case '\\':	g_string_append (result, "\\");	break;
		case '/':	g_string_append (result, "/");	break;
		case '\b':	g_string_append (result, "\b");	break;
		case '\f':	g_string_append (result, "\f");	break;
		case '\n':	g_string_append (result, "\n");	break;
		case '\r':	g_string_append (result, "\r");	break;
		case '\t':	g_string_append (result, "\t");	break;
	}
	g_free (match);
	
	return FALSE;
}

static gchar *
g_json_unescape_string (const gchar *str)
{
	GRegex *regex = g_regex_new ("\\\\(\"|\\\\|/|b|f|n|r|t|u[0-9a-fA-F]{4})",
			0, 0, NULL);
	gchar *result = g_regex_replace_eval (regex, str, -1, 0, 0,
			g_json_unescape_string_eval, NULL, NULL);
	g_regex_unref (regex);
	return result;
}


GJsonNode *
g_json_string_new (const gchar *value)
{
	GJsonNode *node = g_json_node_new ();
	g_json_string_set (node, value);
	return node;
}

const gchar *
g_json_string_get (GJsonNode *node)
{
	if (node && node->type == JSON_TYPE_STRING)
		return node->s;
	return NULL;
}

void
g_json_string_set (GJsonNode *node, const gchar *value)
{
	g_return_if_fail (node != NULL);
	
	if (value) {
		g_json_node_reset (node, JSON_TYPE_STRING);
		node->s = g_strdup (value);
	} else {
		g_json_node_reset (node, JSON_TYPE_NULL);
	}
}



/* Integer */

GJsonNode *
g_json_integer_new (gint64 value)
{
	GJsonNode *node = g_json_node_new ();
	g_json_integer_set (node, value);
	return node;
}

gint64
g_json_integer_get (GJsonNode *node)
{
	switch (node->type) {
	case JSON_TYPE_INTEGER:
		return node->i;
	case JSON_TYPE_FLOAT:
		return (gint64) node->f;
	case JSON_TYPE_STRING:
		return g_ascii_strtoll (node->s, NULL, 10);
	case JSON_TYPE_BOOLEAN:
		return (gint64) node->b;
	default:	// null, array, object
		return 0;
	}
}

void
g_json_integer_set (GJsonNode *node, gint64 value)
{
	g_json_node_reset (node, JSON_TYPE_INTEGER);
	node->i = value;
}



/* Float */

GJsonNode *
g_json_float_new (gdouble value)
{
	GJsonNode *node = g_json_node_new ();
	g_json_float_set (node, value);
	return node;
}

gdouble
g_json_float_get (GJsonNode *node)
{
	switch (node->type) {
	case JSON_TYPE_INTEGER:
		return (gdouble) node->i;
	case JSON_TYPE_FLOAT:
		return node->f;
	case JSON_TYPE_STRING:
		return g_ascii_strtod (node->s, NULL);
	case JSON_TYPE_BOOLEAN:
		return (gint64) node->b;
	default:	// null, array, object
		return 0;
	}
}

void
g_json_float_set (GJsonNode *node, gdouble value)
{
	g_json_node_reset (node, JSON_TYPE_FLOAT);
	node->f = value;
}



/* Boolean */

GJsonNode *
g_json_boolean_new (gboolean value)
{
	GJsonNode *node = g_json_node_new ();
	g_json_boolean_set (node, value);
	return node;
}

gboolean
g_json_boolean_get (GJsonNode *node)
{
	if (node && node->type == JSON_TYPE_BOOLEAN)
		return node->b;
	return FALSE;
}

void
g_json_boolean_set (GJsonNode *node, gboolean value)
{
	g_json_node_reset (node, JSON_TYPE_BOOLEAN);
	node->b = value;
}



/* Array */

GJsonNode *
g_json_array_new ()
{
	GJsonNode *node = g_json_node_new ();
	g_json_node_reset (node, JSON_TYPE_ARRAY);
	return node;
}

void
g_json_array_add (GJsonNode *array, GJsonNode *node)
{
	if (array->type != JSON_TYPE_ARRAY)
		g_json_node_reset (array, JSON_TYPE_ARRAY);
	// if ( not exists )
	g_ptr_array_add (array->a, node);
}

void
g_json_array_remove (GJsonNode *array, GJsonNode *node)
{
	g_return_if_fail (array->type == JSON_TYPE_ARRAY);
	g_ptr_array_remove (array->a, node);
}


static void
g_json_array_list_func (GJsonNode *node, GList **list)
{
	*list = g_list_prepend (*list, node);
}

GList *g_json_array_list (GJsonNode *array)
{
	g_return_val_if_fail (array->type == JSON_TYPE_ARRAY, NULL);
	
	GList *list = NULL;
	g_ptr_array_foreach (array->a, (GFunc) g_json_array_list_func, &list);
	list = g_list_reverse (list);
	return list;
}



/* Object */

GJsonNode *
g_json_object_new ()
{
	GJsonNode *node = g_json_node_new ();
	g_json_node_reset (node, JSON_TYPE_OBJECT);
	return node;
}


gboolean
g_json_object_has (GJsonNode *object, const gchar *name)
{
	g_return_val_if_fail (object->type == JSON_TYPE_OBJECT, FALSE);
	return g_hash_table_lookup_extended (object->o, name, NULL, NULL);
}


void
g_json_object_set (GJsonNode *object, const gchar *name, GJsonNode *node)
{
	if (object->type != JSON_TYPE_OBJECT)
		g_json_node_reset (object, JSON_TYPE_OBJECT);
	g_hash_table_replace (object->o, (gpointer) g_strdup (name), (gpointer) node);
}


GJsonNode *
g_json_object_get (GJsonNode *object, const gchar *name)
{
	g_return_val_if_fail (object->type == JSON_TYPE_OBJECT, NULL);
	return g_hash_table_lookup (object->o, name);
}



/* Iterator */

void
g_json_iter_init (GJsonIter *iter, GJsonNode *node)
{
	g_return_if_fail (node->type == JSON_TYPE_ARRAY ||
			node->type == JSON_TYPE_OBJECT);
	
	iter->node = node;
	if (node->type == JSON_TYPE_ARRAY)
		iter->index = 0;
	else
		g_hash_table_iter_init (&iter->iter, node->o);
}


gboolean
g_json_iter_next_array (GJsonIter *iter, GJsonNode **child)
{
	g_return_val_if_fail (iter->node->type == JSON_TYPE_ARRAY, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	
	if (iter->index >= iter->node->a->len)
		return FALSE;
	
	*child = g_ptr_array_index (iter->node->a, iter->index);
	iter->index++;
	return TRUE;
}


gboolean
g_json_iter_next_object (GJsonIter *iter, gchar **name, GJsonNode **child)
{
	g_return_val_if_fail (iter->node->type == JSON_TYPE_OBJECT, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	
	return g_hash_table_iter_next (&iter->iter, (gpointer *) name, (gpointer *) child);
}

