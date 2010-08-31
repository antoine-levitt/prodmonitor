#include <stdlib.h>
#include <string.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <assert.h>

#include "libwindowswitchs.h"

WnckWindow *current_window = NULL;
char *current_window_name = NULL; //this is a COPY, it has to be created with strdup and free'd
gulong handler_id; // id of the handler for window name change events


void (*enter_callback)(const char* window_name) = NULL;
void (*leave_callback)(const char* window_name) = NULL;


void window_name_change_callback(WnckWindow *win, gpointer user_data)
{
	//printf("Name change callback, %d\n", win);
	if(win) {
		assert(win == current_window);

		leave_callback(current_window_name);

		free(current_window_name);
		current_window_name = strdup(wnck_window_get_name(win));

		if(enter_callback)
			enter_callback(current_window_name);
	}
}

void window_leaved()
{
	if(leave_callback)
		leave_callback(current_window_name);

	g_signal_handler_disconnect(current_window, handler_id);
	handler_id = 0;
	current_window = NULL;
	free(current_window_name);
	current_window_name = NULL;
}

void window_change_callback(WnckScreen *screen, WnckWindow *prev_window, gpointer user_data)
{
	wnck_screen_force_update(screen);
	WnckWindow *active_window = wnck_screen_get_active_window(screen);

	// leaving a window
	if(prev_window) {
		//circumvent wnck bugs: inequality shouldn't happen in
		//theory. If it does, then the window has been deactivated, so we don't have anything to do
		if(current_window == prev_window) {
			window_leaved();
		}
	}

	// entering a window
	if(active_window) {
		if(current_window) {
			// Circumvent wnck bugs ...
			// We haven't been properly disconnected, so disconnect.
			window_leaved();
		}

		current_window = active_window;
		current_window_name = strdup(wnck_window_get_name(current_window));

		if(enter_callback)
			enter_callback(current_window_name);

		//register for name changes
		assert(!handler_id);
		handler_id = g_signal_connect(active_window, "name-changed", G_CALLBACK (window_name_change_callback), NULL);
	}
}
char *getFocusedWindowName()
{
	WnckScreen *screen = wnck_screen_get_default();
	wnck_screen_force_update(screen);
	WnckWindow *window = wnck_screen_get_active_window(screen);
	// do I look like I care about const ?
	return (char *)wnck_window_get_name(window);
}


void windowswitchs_init(void (*enter_window_callback)(const char* window_name), void (*leave_window_callback)(const char* window_name))
{
	int argc = 0;
	char** argv = NULL;
	gtk_init(&argc, &argv);

	enter_callback = enter_window_callback;
	leave_callback = leave_window_callback;
}

void windowswitchs_start()
{
	WnckScreen *screen = wnck_screen_get_default();
	g_signal_connect (screen, "active-window-changed", G_CALLBACK (window_change_callback), NULL);

	gtk_main ();
}

void windowswitchs_stop()
{
	gtk_main_quit();
}
