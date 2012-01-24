#ifndef __GSQUERY__PROTO_SOURCE_H__
#define __GSQUERY__PROTO_SOURCE_H__

#include "querier.h"

void gsq_source_query (GsqQuerier *querier);
gboolean gsq_source_process (GsqQuerier *querier, const gchar *data, gssize size);
void gsq_source_free (GsqQuerier *querier);

#endif