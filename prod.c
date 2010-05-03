//gcc prod.c -o prod -lX11 -lXss -Wall -Wextra $(pkg-config --cflags --libs libwnck-1.0)
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/scrnsaver.h>
#include <time.h>
#include <unistd.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <assert.h>


Display *display;
WnckWindow *current_window = NULL;
char *current_window_name = NULL; //this is a COPY, it has to be created with strdup and free'd
time_t current_window_since;
gulong handler_id; // id of the handler for window name change events
GHashTable *table = NULL;


int error()
{
	printf("error\n");
	exit(1);
}


// from somewhere in Teh Internet. In ms
int getIdleTime () {
        time_t idle_time;
        static XScreenSaverInfo *mit_info;
        Display *display;
        int screen;
        mit_info = XScreenSaverAllocInfo();
        if((display=XOpenDisplay(NULL)) == NULL) { return(-1); }
        screen = DefaultScreen(display);
        XScreenSaverQueryInfo(display, RootWindow(display,screen), mit_info);
        idle_time = (mit_info->idle);
        XFree(mit_info);
        XCloseDisplay(display);
        return idle_time;
}

static void print_element(gpointer key, gpointer val, gpointer user)
{
	int time = GPOINTER_TO_INT(val);
	printf("%4d %s\n", time, key);

}

void print_table()
{
	printf("Table:\n");
	g_hash_table_foreach(table, print_element, NULL);
}

void notice_window_is_inactive()
{
	assert(current_window);
	printf("You spent %ds on %s\n", time(NULL) - current_window_since, current_window_name);
	gpointer res = g_hash_table_lookup(table, current_window_name);
	int new_score = (res ? GPOINTER_TO_INT(res) : 0) + time(NULL) - current_window_since;
	if(res)
		g_hash_table_insert(table, current_window_name, GINT_TO_POINTER(new_score));
	else{
		// have to duplicate, otherwise it will get free'd
		g_hash_table_insert(table, strdup(current_window_name), GINT_TO_POINTER(new_score));
	}
	print_table();
}

void window_name_change_callback(WnckWindow *win, gpointer user_data)
{
	//printf("Name change callback, %d\n", win);
	if(win) {
		assert(win == current_window);
		notice_window_is_inactive();
		free(current_window_name);
		current_window_name = strdup(wnck_window_get_name(win));
		time(&current_window_since);
	}
}
void window_change_callback(WnckScreen *screen, WnckWindow *prev_window, gpointer user_data)
{
	wnck_screen_force_update(screen);
	WnckWindow *active_window = wnck_screen_get_active_window(screen);
	
	/* // unfortunately, due to some weird bug, we can't always assume that the windows won't be NULL */
	/* if(active_window && prev_window) { */
	/* 	printf("Leaving %s for %s\n", wnck_window_get_name(prev_window), wnck_window_get_name(active_window)); */
	/* } */
	/* else */
	/* { */
	/* 	printf("Active %d, prev %d\n", active_window, prev_window); */
	/* } */

	// assume that the event saying that prev_window is no longer
	// active comes BEFORE the one saying that active_window is active

	// leaving a window
	if(prev_window) {
		assert(current_window == prev_window);
		g_signal_handler_disconnect(current_window, handler_id);
		handler_id = 0;
		notice_window_is_inactive();
		current_window = NULL;
		free(current_window_name);
		current_window_name = NULL;
	}
	
	// entering a window
	if(active_window) {
		current_window = active_window;
		current_window_name = strdup(wnck_window_get_name(current_window));
		time(&current_window_since);
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
	return wnck_window_get_name(window);
}


int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	printf("%d\n", getIdleTime());

	WnckScreen *screen = wnck_screen_get_default();
	g_signal_connect (screen, "active-window-changed", G_CALLBACK (window_change_callback), NULL);

	table = g_hash_table_new (g_str_hash, g_str_equal);

	gtk_main ();


	return 0;
}
