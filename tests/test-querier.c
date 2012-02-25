#include "../src/query/querier.h"
#include "../src/query/updater.h"

GMainLoop *loop;


static void
info_updated (GsqQuerier *querier, gpointer udata)
{
	g_print (">> %s, %s, %s, %d / %d, %ld\n", querier->name, querier->game,
			querier->map, querier->numplayers, querier->maxplayers,
			gsq_querier_get_ping (querier));
}

static void
player_updated (GsqQuerier *querier, gpointer udata)
{
	g_print (">> Players %d / %d\n", querier->numplayers, querier->maxplayers);
}

static void
timed_out (GsqQuerier *querier, gpointer udata)
{
	g_print (">> Timed out\n");
}

static void
error_occured (GsqQuerier *querier, const gchar *msg, gpointer udata)
{
	g_print (">> Error: %s\n", msg);
}


static void
sigint ()
{
	g_main_loop_quit (loop);
}


static GsqQuerier *
add_server (GsqUpdater *updater, const gchar *address)
{
	GsqQuerier *querier = gsq_querier_new (address);
	g_signal_connect (querier, "info-update", G_CALLBACK (info_updated), NULL);
	g_signal_connect (querier, "players-update", G_CALLBACK (player_updated), NULL);
	g_signal_connect (querier, "timeout", G_CALLBACK (timed_out), NULL);
	g_signal_connect (querier, "error", G_CALLBACK (error_occured), NULL);
	gsq_updater_take (updater, querier);
	return querier;
}


int
main (int argc, char **argv)
{
	int i;
	GsqUpdater *updater;
	
	if (argc <= 1)
		return 1;
	
	signal (SIGINT, sigint);
	g_type_init ();
	
	loop = g_main_loop_new (NULL, TRUE);
	
	updater = gsq_updater_new ();
	for (i = 1; i < argc; i++)
		add_server (updater, argv[i]);
	
	g_main_loop_run (loop);
	
	g_object_unref (updater);
	
	return 0;
}