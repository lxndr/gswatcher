#ifndef __GTK_POPUP_BUTTON_H__
#define __GTK_POPUP_BUTTON_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS


#define GTK_TYPE_POPUP_BUTTON				(gtk_popup_button_get_type ())
#define GTK_POPUP_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_POPUP_BUTTON, GtkPopupButton))
#define GTK_POPUP_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_POPUP_BUTTON, GtkPopupButtonClass))
#define GTK_IS_POPUP_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_POPUP_BUTTON))
#define GTK_IS_POPUP_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_POPUP_BUTTON))
#define GTK_POPUP_BUTTON_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_POPUP_BUTTON, GtkPopupButtonClass))

typedef struct _GtkPopupButton			GtkPopupButton;
typedef struct _GtkPopupButtonClass		GtkPopupButtonClass;
typedef struct _GtkPopupButtonPrivate	GtkPopupButtonPrivate;

struct _GtkPopupButton {
	GtkToggleButton parent;
	GtkPopupButtonPrivate *priv;
};

struct _GtkPopupButtonClass {
	GtkToggleButtonClass parent_class;
	
	void (*popup) (GtkPopupButton *popup_button);
	void (*popdown) (GtkPopupButton *popup_button);
};


GType gtk_popup_button_get_type (void) G_GNUC_CONST;
GtkWidget *gtk_popup_button_new (const gchar *label);

GtkWidget *gtk_popup_button_get_popup (GtkPopupButton *popup_button);
void gtk_popup_button_popup_for_device (GtkPopupButton *popup_button,
		GdkDevice *device);
void gtk_popup_button_popdown (GtkPopupButton *popup_button);


G_END_DECLS


#endif
