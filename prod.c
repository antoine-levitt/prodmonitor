// prod.c

// Copyright (C) 2010 Antoine Levitt
// Copyright (C) 2010 Thomas Riccardi

// Author: Antoine Levitt
//         Thomas Riccardi <riccardi.thomas@gmail.com>
// URL: http://github.com/antoine-levitt/prodmonitor/tree/sqlite

// This program is free software. It comes without any warranty, to
// the extent permitted by applicable law. You can redistribute it
// and/or modify it under the terms of the Do What The Fuck You Want
// To Public License, Version 2, as published by Sam Hocevar. See
// http://sam.zoy.org/wtfpl/COPYING for more details.


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <glib.h>

#include "libwindowswitchs.h"

#include <sqlite3.h>
#include <signal.h>
#include <assert.h>

#ifndef DEBUG
#define DEBUG 0
#endif

int quiet = 0;

/**
 * TODO:
 *  * remove hardoded values, add program parameters
 *  * remove display code (hashtable and all): this is just a recording program
 *  * record milliseconds?
 */

/*
sqlite commands tests, from ".d" command in sqlite3

PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE stats (id INTEGER PRIMARY KEY, title TEXT, start INT, stop INT);
INSERT INTO "stats" VALUES(1,'emacs',1272995780,1272995787);
INSERT INTO "stats" VALUES(2,'iceweasel',1272995787,1272995795);
COMMIT;

*/

time_t current_window_since;
GHashTable *table = NULL;

sqlite3 *db;
char *db_table = "stats";


/* replace each character by replacement in a copy of str */
char* char_replace(const char *str, const char character, const char *replacement)
{
	size_t replacement_size = strlen(replacement);
	size_t buffer_size = strlen(str)*replacement_size + 1; // max size, if each char of str is character
	char *buffer = (char*) malloc(sizeof(char)*buffer_size);

	const char *str_cursor = str;
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

int db_create_table_if_not_exists()
{
	int rc;
	char *zErrMsg = 0;

	// construct the sql query
	char *sql_template = "CREATE TABLE IF NOT EXISTS %s (id INTEGER PRIMARY KEY, title TEXT, start INT, stop INT);";

	size_t size = strlen(sql_template) - 2 + strlen(db_table) + 1;
	char *sql = (char*) malloc(sizeof(char)*size);
	rc = snprintf(sql, size, sql_template, db_table);
	assert(rc < (int)size);// if not, the size calculation is wrong
	if (rc<0) {
		fprintf(stderr, "ERROR creating sql query. snprintf returned %d\n", rc);
	}

	// run it
#if DEBUG
	printf("Executing sqlite query: %s\n", sql);
#endif
	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
#if DEBUG
	printf("Result: %d\n", rc);
#endif
	if (rc != SQLITE_OK) {
		fprintf(stderr, "ERROR executing sql query \"%s\". sqlite3_exec returned %d. Error: %s\n", sql, rc, zErrMsg);
		sqlite3_free(zErrMsg);
	}

	free(sql);

	return rc;
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
	//printf("You spent %lds on %s\n", time(NULL) - current_window_since, window_name);

	gpointer res = g_hash_table_lookup(table, name);
	int new_score = (res ? GPOINTER_TO_INT(res) : 0) + time(NULL) - current_window_since;

	g_hash_table_insert(table, name, GINT_TO_POINTER(new_score));
	// if there was already a key, we did the copy for nothing
	if(res)
		free(name);
	if (! quiet) print_table();


	/* insert in db */
	int rc;
	char *zErrMsg = 0;

	// construct the sql query
	char *sql_template = "INSERT INTO \"stats\" (title, start, stop) VALUES ('%s',%u, %u);";
	// escape ' in window_name
	char *escaped_window_name = char_replace(window_name, '\'', "''");

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
#if DEBUG
	printf("Result: %d\n", rc);
#endif
	if (rc != SQLITE_OK) {
		fprintf(stderr, "ERROR executing sql query \"%s\". sqlite3_exec returned %d. Error: %s\n", sql, rc, zErrMsg);
		sqlite3_free(zErrMsg);
	}
	free(sql);
}

void sigquit_handler(int sig)
{
	windowswitchs_stop();
}


int main(int argc, char *argv[])
{
	quiet = (argc > 1 && !strcmp(argv[1], "-q"));

	// init libwindowswitchs
	windowswitchs_init(&enter_window, &leave_window);

	/* register signal handlers, to properly quit */
	(void) signal(SIGINT,sigquit_handler);
	(void) signal(SIGQUIT,sigquit_handler);
	(void) signal(SIGTERM,sigquit_handler);

	table = g_hash_table_new (g_str_hash, g_str_equal);
	time(&current_window_since);

	/* open sqlite database */
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_open("test.db", &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}


	rc = db_create_table_if_not_exists();
	if (rc != SQLITE_OK) {
		// no table, and not able to create it
		return rc;
	}

	if (!quiet) printf("Start\n");

	windowswitchs_start();


	/* quit */
	/* close db */
	sqlite3_close(db);

	if (!quiet) printf("End\n");

	return 0;
}
