#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <glib.h>

#include "libwindowswitchs.h"


time_t current_window_since;
GHashTable *table = NULL;


int error()
{
	printf("error\n");
	exit(1);
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

	printf("\n");
	printf("--------------------------------------------------------------------------------\n");
	for(i = 0;i<N;i++)
	{
		printf("%4d %s\n", pair_arr[i].value, pair_arr[i].key);
	}

	free(pair_arr);
}

//if the name of the window contains one of these strings, it is
//replaced by it. Avoids having a lot of entries for similar activities
char *contract_names_list[] = {
	"Slashdot",
	"Google Reader",
	"Facebook",
	"Google Chrome",
	"Amarok",
	".m - Emacs",
	".tex - Emacs"
};
//return a new string, post-processed
char *post_process_names(const char *win_name)
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

void enter_window(const char* window_name)
{
	// start time for this new window
	time(&current_window_since);
}

void leave_window(const char* window_name)
{
	char *name = post_process_names(window_name);
	//printf("You spent %lds on %s\n", time(NULL) - current_window_since, current_window_name);

	gpointer res = g_hash_table_lookup(table, name);
	int new_score = (res ? GPOINTER_TO_INT(res) : 0) + time(NULL) - current_window_since;

	g_hash_table_insert(table, name, GINT_TO_POINTER(new_score));
	// if there was already a key, we did the copy for nothing
	if(res)
		free(name);
	print_table();
}

void sigquit_handler(int sig)
{
	windowswitchs_stop();
}

int main(int argc, char *argv[])
{
	// init libwindowswitchs
	windowswitchs_init(&enter_window, &leave_window);

	/* register signal handlers, to properly quit */
	(void) signal(SIGINT,sigquit_handler);
	(void) signal(SIGQUIT,sigquit_handler);
	(void) signal(SIGTERM,sigquit_handler);

	table = g_hash_table_new (g_str_hash, g_str_equal);

	time(&current_window_since);

	printf("Start\n");

	windowswitchs_start();

	printf("End\n");

	return 0;
}
