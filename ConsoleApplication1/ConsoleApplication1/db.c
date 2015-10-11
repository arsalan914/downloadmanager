#include "db.h"

int open_db(char * db_name, sqlite3 **db)
{
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open(db_name, db);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		exit(0);
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

}

int create_table(char *sql, sqlite3 *db)
{
	char *zErrMsg = 0;
	int rc;

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	else
	{
		fprintf(stdout, "Operation done successfully\n");
	}

	return 0;
}

int insert_in_table(sqlite3 *db, char *fn, int thread, int start, int end, int remain)
{
	char sql[500];
	char *zErrMsg = 0;
	int rc;

	sprintf(sql, "INSERT INTO mdm_info (filename,thread,start,end,remain) VALUES ('%s', %d, %d, %d, %d ); ", fn, thread, start, end, remain);


	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		;// fprintf(stdout, "Operation done successfully\n");
	}

}

static int callback(void *data, int argc, char **argv, char **azColName){
	int i;
//	fprintf(stderr, "%s: ", (const char*)data);
	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

static int fill_data(void *data, int argc, char **argv, char **azColName){
	int i;
//	fprintf(stderr, "%s: ", (const char*)data);

	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(azColName[i], "start") == 0)
		{
			((db_schema *)data)->start = atoi(argv[i]);
		}
		else if (strcmp(azColName[i], "end") == 0)
		{
			((db_schema *)data)->end = atoi(argv[i]);
		}
		else if (strcmp(azColName[i], "remain") == 0)
		{
			((db_schema *)data)->remaining = atoi(argv[i]);
		}
	}
	if (argc <= 0)
	{
		memset(data, -1, sizeof(db_schema));
	}

	printf("\n");
	return 0;
}

int read_all_table(sqlite3 *db)
{
	char *zErrMsg = 0;
	int rc;
	char *sql;

	/* Create SQL statement */
	sql = "SELECT * from mdm_info";

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else{
		fprintf(stdout, "Operation done successfully\n");
	}
}

int update_entry(sqlite3 *db, char *fn, int thread, int remain)
{
	char *zErrMsg = 0;
	int rc;
	char sql[400];

	/* Create merged SQL statement */
	sprintf(sql, "UPDATE mdm_info set remain = %d where thread=%d AND filename LIKE '%s';", remain, thread, fn);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0,0, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		;// fprintf(stdout, "Operation done successfully\n");
	}
}

int get_thread_details(sqlite3 *db, db_schema *data)
{
	char *zErrMsg = 0;
	int rc;
	char sql[400];

	/* Create merged SQL statement */
	sprintf(sql, "select * from mdm_info  where thread=%d AND filename LIKE '%s';", data->thread_number, data->filename);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, fill_data, (void *)data, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else{
		fprintf(stdout, "Operation done successfully\n");
	}

}

