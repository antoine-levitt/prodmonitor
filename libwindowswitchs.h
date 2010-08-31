// libwindowswitch.h -- Get window switchs

/*
 * Callback functions prototype:
 * Called with the window's name
 */
typedef void (*callback_function)(const char* window_name);

/*
 * Init WindowSwitchs with the two callback functions:
 *  called respectively at enter and when leaving a window.
 * Callbacks can be NULL, to deactivate
 * Also init gtk
 */
void windowswitchs_init(callback_function enter_window_callback, callback_function leave_window_callback);

/*
 * Start WindowSwitchs
 * Returns only when windowswitchs_stop() called
 */
void windowswitchs_start();

/*
 * Stop WindowSwitchs
 */
void windowswitchs_stop();
