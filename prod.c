//gcc prod.c -o prod -Wall -Wextra -Wno-unused $(pkg-config --cflags --libs libwnck-1.0) -llibsqlite3
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <assert.h>

#include <sqlite3.h>

#ifndef DEBUG
#define DEBUG 0
#endif

/*
sqlite commands tests, from ".d" command in sqlite3

PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE stats (id INTEGER PRIMARY KEY, title TEXT, start INT, stop INT);
INSERT INTO "stats" VALUES(1,'emacs',1272995780,1272995787);
INSERT INTO "stats" VALUES(2,'iceweasel',1272995787,1272995795);
COMMIT;

*/

WnckWindow *current_window = NULL;
char *current_window_name = NULL; //this is a COPY, it has to be created with strdup and free'd
time_t current_window_since;
gulong handler_id; // id of the handler for window name change events
GHashTable *table = NULL;

sqlite3 *db;

int error()
{
	printf("error\n");
	exit(1);
}

/* replace each character by replacement in a copy of str */
char* char_replace(char *str, char character, char *replacement)
{
	size_t replacement_size = strlen(replacement);
	size_t buffer_size = strlen(str)*replacement_size + 1; // max size, if each char of str is character
	char *buffer = (char*) malloc(sizeof(char)*buffer_size);

	char *str_cursor = str;
	char *buffer_cursor = buffer;
	char *new_str_cursor;
	while ((new_str_cursor = strchr(str_cursor, character))) {
		// got a character to replace
		// copy to buffer part of str before the character
		memcpy(buffer_cursor, str_cursor, new_str_cursor-str_cursor);
		buffer_cursor += new_str_cursor-str_cursor;
		// copy the replacement string
		strcpy(buffer_cursor, replacement);
		buffer_cursor += replacement_size;

		str_cursor = new_str_cursor + 1;
	}

	// copy remaining part
	strcpy(buffer_cursor, str_cursor);

	return buffer;
}

struct pair {
	char *key;
	int value;
};

int compare_pairs(const void *p1, const void *p2)
{
	return ((struct pair *)p2)->value - ((struct pair *)p1)->value;
}

void print_table()
{
	/* printf("Table:\n"); */
	/* g_hash_table_foreach(table, print_element, NULL); */

	int N = g_hash_table_size(table);
	struct pair *pair_arr = malloc(sizeof(struct pair) * N);

	GHashTableIter iter;
	gpointer key, value;

	int i = 0;
	g_hash_table_iter_init (&iter, table);
	while (g_hash_table_iter_next (&iter, &key, &value)){
		struct pair zepair;
		zepair.key = (char *) key;
		zepair.value = GPOINTER_TO_INT(value);
		pair_arr[i++] = zepair;
	}

	qsort(pair_arr, N, sizeof(struct pair), compare_pairs);

	printf("--------------------------------------------------------------------------------\n");
	for(i = 0;i<N;i++)
	{
		printf("%4d %s\n", pair_arr[i].value, pair_arr[i].key);
	}
	printf("\n");

	free(pair_arr);
}

//if the name of the window contains one of these strings, it is
//replaced by it. Avoids having a lot of entries for similar activities
char *contract_names_list[] = {
	"Slashdot",
	"Google Reader",
	"Facebook",
	".m - Emacs",
	".tex - Emacs"
};
//return a new string, post-processed
char *post_process_names(char *win_name)
{
	int i;
	int N = sizeof(contract_names_list)/sizeof(*contract_names_list);
	for(i = 0; i < N;i++) {
		if(strstr(win_name, contract_names_list[i])) {
			return strdup(contract_names_list[i]);
		}
	}
	return strdup(win_name);
}

void notice_window_is_inactive()
{
	assert(current_window);
	char *name = post_process_names(current_window_name);
	//printf("You spent %lds on %s\n", time(NULL) - current_window_since, current_window_name);

	gpointer res = g_hash_table_lookup(table, name);
	int new_score = (res ? GPOINTER_TO_INT(res) : 0) + time(NULL) - current_window_since;
	g_hash_table_insert(table, name, GINT_TO_POINTER(new_score));
	// if there was already a key, we did the copy for nothing
	if(res)
		free(name);
	print_table();

	/* insert in db */
	int rc;
	char *zErrMsg = 0;

	// construct the sql query
	char *sql_template = "INSERT INTO \"stats\" (title, start, stop) VALUES('%s',%u, %u);";
	// escape ' in current_window_name
	char *escaped_window_name = char_replace(current_window_name, '\'', "''");

	size_t size = strlen(sql_template) - 3*2 + strlen(escaped_window_name) + 2*10 + 1; // 2 unsigned int in decimal form
	char *sql = (char*) malloc(sizeof(char)*size);
	rc = snprintf(sql, size, sql_template, escaped_window_name, (unsigned int)current_window_since, (unsigned int)time(NULL));
	assert(rc < (int)size);// if not, the size calculation is wrong
	if (rc<0) {
		fprintf(stderr, "ERROR creating sql query. snprintf returned %d\n", rc);
	}
	free(escaped_window_name);

	// run it
#if DEBUG
	printf("Executing sqlite query: %s\n", sql);
#endif
	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
	if (rc<0) {
		fprintf(stderr, "ERROR executing sql query \"%s\". sqlite3_exec returned %d\n", sql, rc);
	}
	free(sql);
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

void window_leaved()
{
	g_signal_handler_disconnect(current_window, handler_id);
	handler_id = 0;
	notice_window_is_inactive();
	current_window = NULL;
	free(current_window_name);
	current_window_name = NULL;
}

void window_change_callback(WnckScreen *screen, WnckWindow *prev_window, gpointer user_data)
{
	wnck_screen_force_update(screen);
	WnckWindow *active_window = wnck_screen_get_active_window(screen);

#if DEBUG
	//debug
	if(active_window && prev_window) {
		printf("Leaving %s for %s\n", wnck_window_get_name(prev_window), wnck_window_get_name(active_window));
	}
	else
	{
		printf("Active %d, prev %d\n", active_window, prev_window);
	}
#endif

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
	// do I look like I care about const ?
	return (char *)wnck_window_get_name(window);
}


int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	WnckScreen *screen = wnck_screen_get_default();
	g_signal_connect (screen, "active-window-changed", G_CALLBACK (window_change_callback), NULL);

	table = g_hash_table_new (g_str_hash, g_str_equal);

	/* open sqlite database */
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_open("test.db", &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	gtk_main ();

	// TODO : close db on quit
	//sqlite3_close(db);

	return 0;
}
