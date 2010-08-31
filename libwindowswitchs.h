// libwindowswitch.h -- Get window switchs

typedef void (*callback_function)(const char* window_name);
//TODO use this

/*
 * Init WindowSwitchs with the two callback functions:
 *  called respectively at enter and when leaving a window.
 *  unique parameter: the window's name
 * Callbacks can be NULL, to deactivate
 * Also init gtk
 */
void windowswitchs_init(void (*enter_window_callback)(const char* window_name), void (*leave_window_callback)(const char* window_name));

/*
 * Start WindowSwitchs
 * Returns only when called windowswitchs_stop()
 */
void windowswitchs_start();

/*
 * Stop WindowSwitchs
 */
void windowswitchs_stop();
