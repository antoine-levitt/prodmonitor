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


Display *display;
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

void window_change_callback(WnckScreen *screen, WnckWindow *prev_window, gpointer user_data)
{
	// unfortunately, due to some weird bug, we can't always assume that the windows won't be NULL
	printf("Event called\n");
	wnck_screen_force_update(screen);
	WnckWindow *active_window = wnck_screen_get_active_window(screen);
	if(active_window && prev_window) {
		printf("Leaving %s for %s\n", wnck_window_get_name(prev_window), wnck_window_get_name(active_window));
	}
	else
	{
		printf("Fail. Active %d, prev %d\n", active_window, prev_window);
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
	gtk_main ();

	return 0;
}
