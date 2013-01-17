#include "gtkpopupbutton.h"


enum {
	SIGNAL_POPUP,
	SIGNAL_POPDOWN,
	LAST_SIGNAL
};


struct _GtkPopupButtonPrivate {
	GtkWidget *dock;
	GdkDevice *grab_pointer;
	GdkDevice *grab_keyboard;
};


static guint signals[LAST_SIGNAL] = {0};


static void gtk_popup_button_dispose(GObject* object);
static void gtk_popup_button_popup_real (GtkPopupButton *popup_button, GdkDevice *device);
static void gtk_popup_button_popdown_real (GtkPopupButton *popup_button);
static gboolean gtk_popup_button_press_event (GtkWidget *widget,
		GdkEventButton *event);
static gboolean dock_button_press_event (GtkWidget *widget,
		GdkEventButton *event, gpointer udata);
static gboolean dock_key_release_event (GtkWidget *widget,
		GdkEventKey *event, gpointer udata);


G_DEFINE_TYPE (GtkPopupButton, gtk_popup_button, GTK_TYPE_TOGGLE_BUTTON);


static void
gtk_popup_button_class_init (GtkPopupButtonClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	object_class->dispose = gtk_popup_button_dispose;
	g_type_class_add_private (object_class, sizeof (GtkPopupButtonPrivate));
	
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	widget_class->button_press_event = gtk_popup_button_press_event;
	
	signals[SIGNAL_POPUP] = g_signal_new_class_handler ("popup",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			G_CALLBACK (gtk_popup_button_popup_real), NULL, NULL,
			g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GDK_TYPE_DEVICE);
	
	signals[SIGNAL_POPDOWN] = g_signal_new_class_handler ("popdown",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			G_CALLBACK (gtk_popup_button_popdown_real), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
gtk_popup_button_init (GtkPopupButton *popup_button)
{
	popup_button->priv = G_TYPE_INSTANCE_GET_PRIVATE (popup_button,
			GTK_TYPE_POPUP_BUTTON, GtkPopupButtonPrivate);
	GtkPopupButtonPrivate *priv = popup_button->priv;
	
	/* popup window */
	priv->dock = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_container_set_border_width (GTK_CONTAINER (priv->dock), 4);
	gtk_widget_set_name (priv->dock, "gtk-popupbutton-popup-window");
	gtk_window_set_decorated (GTK_WINDOW (priv->dock), FALSE);
	g_signal_connect (priv->dock, "button-press-event",
			G_CALLBACK (dock_button_press_event), popup_button);
	g_signal_connect (priv->dock, "key-release-event",
			G_CALLBACK (dock_key_release_event), popup_button);
}


GtkWidget *
gtk_popup_button_new ()
{
	return g_object_new (GTK_TYPE_POPUP_BUTTON, NULL);
}


static void
gtk_popup_button_dispose (GObject* object)
{
	GtkPopupButton *popup_button = GTK_POPUP_BUTTON (object);
	
	if (popup_button->priv->dock) {
		gtk_widget_destroy (popup_button->priv->dock);
		popup_button->priv->dock = NULL;
	}
	
	G_OBJECT_CLASS (gtk_popup_button_parent_class)->dispose (object);
}


GtkWidget *
gtk_popup_button_get_popup (GtkPopupButton *popup_button)
{
	g_return_val_if_fail (GTK_IS_POPUP_BUTTON (popup_button), NULL);
	return popup_button->priv->dock;
}


void
gtk_popup_button_popup (GtkPopupButton *popup_button)
{
	GdkDevice *device = gtk_get_current_event_device ();
	
	if (!device) {
		GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (popup_button));
		GdkDeviceManager *device_manager = gdk_display_get_device_manager (display);
		GList *devices = gdk_device_manager_list_devices (
				device_manager, GDK_DEVICE_TYPE_MASTER);
		device = devices->data;
		g_list_free (devices);
	}
	
	gtk_popup_button_popup_for_device (popup_button, device);
}


void
gtk_popup_button_popup_for_device (GtkPopupButton *popup_button, GdkDevice *device)
{
	g_return_if_fail (GTK_IS_POPUP_BUTTON (popup_button));
	g_signal_emit (popup_button, signals[SIGNAL_POPUP], 0, device);
}


void
gtk_popup_button_popdown (GtkPopupButton *popup_button)
{
	g_return_if_fail (GTK_IS_POPUP_BUTTON (popup_button));
	g_signal_emit (popup_button, signals[SIGNAL_POPDOWN], 0);
}


static gboolean
gtk_popup_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	gtk_popup_button_popup_for_device (GTK_POPUP_BUTTON (widget), event->device);
	return TRUE;
}


static gboolean
dock_button_press_event (GtkWidget *widget, GdkEventButton *event,
		gpointer udata)
{
	if (event->type == GDK_BUTTON_PRESS) {
		GtkPopupButton *btn = GTK_POPUP_BUTTON (udata);
		gint dock_x, dock_y, dock_w, dock_h;
		
		gtk_window_get_position (GTK_WINDOW (btn->priv->dock), &dock_x, &dock_y);
		gtk_window_get_size (GTK_WINDOW (btn->priv->dock), &dock_w, &dock_h);
		if ((event->x_root < dock_x || event->x_root > dock_x + dock_w) ||
				(event->y_root < dock_y || event->y_root > dock_y + dock_h))
			gtk_popup_button_popdown (btn);
		
		return TRUE;
	}
	
	return FALSE;
}


static gboolean
dock_key_release_event (GtkWidget *widget, GdkEventKey *event, gpointer udata)
{
	if (event->keyval == GDK_KEY_Escape)
		gtk_popup_button_popdown (GTK_POPUP_BUTTON (udata));
	return TRUE;
}


static void
gtk_popup_button_popup_real (GtkPopupButton *popup_button, GdkDevice *device)
{
	GtkPopupButtonPrivate *priv = popup_button->priv;
	GtkAllocation button_allocation, dock_allocation;
	GdkScreen *screen;
	gint monitor_num;
	GdkRectangle monitor;
	GtkWidget *toplevel;
	GdkDevice *keyboard, *pointer;
	guint32 time;
	gint x, y;
	
	g_return_if_fail (GTK_IS_POPUP_BUTTON (popup_button));
	g_return_if_fail (GDK_IS_DEVICE (device));
	
	if (!gtk_widget_get_realized (GTK_WIDGET (popup_button)))
		return;
	
	if (gtk_widget_get_mapped (priv->dock))
		return;
	
	if (priv->grab_pointer && priv->grab_keyboard)
		return;
	
	time = gtk_get_current_event_time ();
	
	if (gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD) {
		keyboard = device;
		pointer = gdk_device_get_associated_device (device);
	} else {
		pointer = device;
		keyboard = gdk_device_get_associated_device (device);
	}
	
	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (popup_button));
	if (GTK_IS_WINDOW (toplevel))
		gtk_window_group_add_window (gtk_window_get_group (GTK_WINDOW (toplevel)),
				GTK_WINDOW (priv->dock));
	
	/* positioning */
	gtk_widget_show (priv->dock);
	gtk_widget_get_allocation (GTK_WIDGET (popup_button), &button_allocation);
	gdk_window_get_root_coords (gtk_widget_get_window (GTK_WIDGET (popup_button)),
			button_allocation.x, button_allocation.y, &x, &y);
	gtk_widget_get_allocation (GTK_WIDGET (priv->dock), &dock_allocation);
	
	x = (x + button_allocation.width / 2) - (dock_allocation.width / 2);
	y = y - dock_allocation.height;
	
	GdkWindow *window = window = gtk_widget_get_window (GTK_WIDGET (popup_button));
	screen = gtk_widget_get_screen (GTK_WIDGET (priv->dock));
	monitor_num = gdk_screen_get_monitor_at_window (screen, window);
	gdk_screen_get_monitor_workarea (screen, monitor_num, &monitor);
	
	if (x < monitor.x)
		x = monitor.x;
	else if (x + dock_allocation.width > monitor.x + monitor.width)
		x = monitor.x + monitor.width - dock_allocation.width;
	
	gtk_window_move (GTK_WINDOW (priv->dock), x, y);
	
	/* grabbing */
	gtk_widget_grab_focus (priv->dock);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (popup_button), TRUE);
	
	if (gdk_device_grab (pointer, gtk_widget_get_window (priv->dock),
			GDK_OWNERSHIP_WINDOW, TRUE,
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
			NULL, time) != GDK_GRAB_SUCCESS) {
		gtk_device_grab_remove (priv->dock, pointer);
		gtk_widget_hide (priv->dock);
		return;
	}
	
	if (gdk_device_grab (keyboard, gtk_widget_get_window (priv->dock),
			GDK_OWNERSHIP_WINDOW, TRUE,
			GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
			NULL, time) != GDK_GRAB_SUCCESS) {
		gdk_device_ungrab (pointer, time);
		gtk_device_grab_remove (priv->dock, pointer);
		gtk_widget_hide (priv->dock);
		return;
	}
	
	gtk_device_grab_add (priv->dock, pointer, TRUE);
	priv->grab_pointer = pointer;
	priv->grab_keyboard = keyboard;
}


static void
gtk_popup_button_popdown_real (GtkPopupButton *popup_button)
{
	GtkPopupButtonPrivate *priv = popup_button->priv;
	
	g_return_if_fail (GTK_IS_POPUP_BUTTON (popup_button));
	
	if (!gtk_widget_get_realized (GTK_WIDGET (popup_button)))
		return;
	
	gdk_device_ungrab (priv->grab_keyboard, GDK_CURRENT_TIME);
	gdk_device_ungrab (priv->grab_pointer, GDK_CURRENT_TIME);
	gtk_device_grab_remove (priv->dock, priv->grab_pointer);
	gtk_widget_hide (priv->dock);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (popup_button), FALSE);
	
	priv->grab_pointer = NULL;
	priv->grab_keyboard = NULL;
}
