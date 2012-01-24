#ifndef __GSQUERY__QUERIER_PRIVATE_H__
#define __GSQUERY__QUERIER_PRIVATE_H__

#include "querier.h"

G_BEGIN_DECLS


#define GSQ_LOG_DOMAIN "GSQuery"


// protocol private data
void gsq_querier_set_pdata (GsqQuerier *querier, gpointer pdata);
gpointer gsq_querier_get_pdata (GsqQuerier *querier);

void gsq_querier_send (GsqQuerier *querier, guint16 port, const gchar *data,
		gsize length);
void gsq_querier_add_player (GsqQuerier *querier, const gchar *name, gint kills,
		const gchar *time);

void gsq_querier_set_name (GsqQuerier *querier, const gchar *value);
void gsq_querier_set_game (GsqQuerier *querier, const gchar *value);
void gsq_querier_set_map (GsqQuerier *querier, const gchar *value);
void gsq_querier_set_version (GsqQuerier *querier, const gchar *value);
void gsq_querier_set_extra (GsqQuerier *querier, const gchar *key,
		const gchar *value);


G_END_DECLS

#endif
