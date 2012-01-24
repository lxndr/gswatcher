#include "gui-window.h"
#include "gui-log.h"


static GtkWidget *logview;
static GtkWidget *enable_log;


GtkWidget *
gs_log_create_bar ()
{
	enable_log = gtk_check_button_new_with_mnemonic (_("enable _log"));
	
	GtkWidget *bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (bar), enable_log, FALSE, TRUE, 0);
	gtk_widget_show_all (bar);
	
	return bar;
}


GtkWidget *
gs_log_create ()
{
	logview = gtk_text_view_new ();
	g_object_set (G_OBJECT (logview),
			"editable", FALSE,
			"wrap-mode", GTK_WRAP_WORD,
			NULL);
	
	GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (G_OBJECT (scrolled),
			"shadow-type", GTK_SHADOW_IN,
			"hscrollbar-policy", GTK_POLICY_NEVER,
			"vscrollbar-policy", GTK_POLICY_ALWAYS,
			NULL);
	gtk_container_add (GTK_CONTAINER (scrolled), logview);
	gtk_widget_show_all (scrolled);
	
	return scrolled;
}