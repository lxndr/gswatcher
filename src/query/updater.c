/* 
 * updater.c
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



#include "updater.h"


enum {
	PROP_0,
	PROP_RATE,
	PROP_PAUSE,
};

struct _GsqUpdaterPrivate {
	GList *queriers;
	GList *iter;
	guint timer;
	gfloat rate;
	gboolean paused;
};


G_DEFINE_TYPE (GsqUpdater, gsq_updater, G_TYPE_OBJECT);


static void
gsq_updater_finalize (GObject *object)
{
	GsqUpdater *updater = GSQ_UPDATER (object);
	GsqUpdaterPrivate *priv = updater->priv;
	
	if (priv->timer)
		g_source_remove (priv->timer);
	
	while (priv->queriers) {
		g_object_unref (priv->queriers->data);
		priv->queriers = g_list_delete_link (priv->queriers, priv->queriers);
	}
}

static void
gsq_updater_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsqUpdater *updater = GSQ_UPDATER (object);
	
	switch (prop_id) {
	case PROP_RATE:
		gsq_updater_set_rate (updater, g_value_get_float (value));
		break;
	case PROP_PAUSE:
		gsq_updater_set_pause (updater, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gsq_updater_get_property (GObject *object, guint prop_id, GValue *value,
		GParamSpec *pspec)
{
	GsqUpdater *updater = GSQ_UPDATER (object);
	
	switch (prop_id) {
	case PROP_RATE:
		g_value_set_float (value, gsq_updater_get_rate (updater));
		break;
	case PROP_PAUSE:
		g_value_set_boolean (value, gsq_updater_get_pause (updater));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}

static void
gsq_updater_class_init (GsqUpdaterClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->set_property = gsq_updater_set_property;
	gobject_class->get_property = gsq_updater_get_property;
	gobject_class->finalize = gsq_updater_finalize;
	
	g_type_class_add_private (gobject_class, sizeof (GsqUpdaterPrivate));
	
	g_object_class_install_property (gobject_class, PROP_RATE,
			g_param_spec_float ("rate", "Update rate", "Update rate", 0.5f, 30.0f, 2.5f,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	
	g_object_class_install_property (gobject_class, PROP_PAUSE,
			g_param_spec_boolean ("pause", "Pause", "Pause", FALSE,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}

static void
gsq_updater_init (GsqUpdater *updater)
{
	updater->priv = G_TYPE_INSTANCE_GET_PRIVATE (updater, GSQ_TYPE_UPDATER,
			GsqUpdaterPrivate);
}

GsqUpdater *
gsq_updater_new ()
{
	return g_object_new (GSQ_TYPE_UPDATER, NULL);
}


static gboolean
timer_tick (GsqUpdater *updater)
{
	GsqUpdaterPrivate *priv = updater->priv;
	
	if (priv->paused || !priv->queriers)
		return TRUE;
	
	if (priv->iter == NULL)
		priv->iter = priv->queriers;
	
	gsq_querier_update ((GsqQuerier *) priv->iter->data);
	priv->iter = g_list_next (priv->iter);
	
	return TRUE;
}

static void
update_timer (GsqUpdater *updater)
{
	GsqUpdaterPrivate *priv = updater->priv;
	
	if (priv->timer) {
		g_source_remove (priv->timer);
		priv->timer = 0;
	}
	
	guint count = g_list_length (priv->queriers);
	if (count > 0)
		priv->timer = g_timeout_add (priv->rate * 1000 / count,
				(GSourceFunc) timer_tick, updater);
}

void
gsq_updater_set_rate (GsqUpdater *updater, gfloat rate)
{
	g_return_if_fail (GSQ_IS_UPDATER (updater));
	updater->priv->rate = rate;
	update_timer (updater);
	g_object_notify (G_OBJECT (updater), "rate");
}

gfloat
gsq_updater_get_rate (GsqUpdater *updater)
{
	g_return_val_if_fail (GSQ_IS_UPDATER (updater), 0.f);
	return updater->priv->rate;
}

void
gsq_updater_set_pause (GsqUpdater *updater, gboolean pause)
{
	g_return_if_fail (GSQ_IS_UPDATER (updater));
	updater->priv->paused = pause;
	g_object_notify (G_OBJECT (updater), "pause");
}

gboolean
gsq_updater_get_pause (GsqUpdater *updater)
{
	g_return_val_if_fail (GSQ_IS_UPDATER (updater), FALSE);
	return updater->priv->paused;
}


void
gsq_updater_add (GsqUpdater *updater, GsqQuerier *querier)
{
	g_return_if_fail (GSQ_IS_UPDATER (updater));
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	
	GsqUpdaterPrivate *priv = updater->priv;
	priv->queriers = g_list_append (priv->queriers, g_object_ref (querier));
	update_timer (updater);
}

void
gsq_updater_take (GsqUpdater *updater, GsqQuerier *querier)
{
	g_return_if_fail (GSQ_IS_UPDATER (updater));
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	
	GsqUpdaterPrivate *priv = updater->priv;
	priv->queriers = g_list_append (priv->queriers, querier);
	update_timer (updater);
}

void
gsq_updater_remove (GsqUpdater *updater, GsqQuerier *querier)
{
	g_return_if_fail (GSQ_IS_UPDATER (updater));
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	
	GsqUpdaterPrivate *priv = updater->priv;
	priv->queriers = g_list_remove (priv->queriers, querier);
	g_object_unref (priv->queriers->data);
	update_timer (updater);
}

GList *
gsq_updater_list (GsqUpdater *updater)
{
	g_return_val_if_fail (GSQ_IS_UPDATER (updater), NULL);
	return g_list_copy (updater->priv->queriers);
}
