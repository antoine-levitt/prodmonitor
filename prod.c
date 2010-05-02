// needs libxss-dev and libx11-dev
//compile with -lX11 -lXss
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/scrnsaver.h>
#include <time.h>
#include <unistd.h>

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
char *getWindowName(Window win)
{
	XTextProperty text_prop;
	if(!XGetWMName(display, win, &text_prop))
	{
		// go up
		Window rroot, rparent, *rchildren;
		unsigned int rnum;
		XQueryTree(display, win, &rroot, &rparent, &rchildren, &rnum);
		char *result = getWindowName(rparent);
		XFree(rchildren);
		return result;
	}
	else
	{
		return (char *)text_prop.value;
	}
}
char *getFocusedWindowName()
{
	Window focus;
	int revert;

	XGetInputFocus(display, &focus, &revert);
	return getWindowName(focus);
}


int main()
{
	display = XOpenDisplay(NULL);

	printf("%s\n", getFocusedWindowName());
	printf("%d\n", getIdleTime());
	return 0;
}
