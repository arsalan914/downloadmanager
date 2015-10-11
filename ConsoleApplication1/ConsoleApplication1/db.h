#include "sqlite3.h"
#include "defines.h"
#include <stdio.h>
#include <string.h>


int open_db(char * db_name, sqlite3 **db);
#define close_db(db) sqlite3_close(db)
int create_table(char *sql, sqlite3 *db);
int insert_in_table(sqlite3 *db, char *fn, int thread, int start, int end, int remain);
int update_entry(sqlite3 *db, char *fn, int thread, int remain);
int read_all_table(sqlite3 *db);
int get_thread_details(sqlite3 *db, db_schema *data);
