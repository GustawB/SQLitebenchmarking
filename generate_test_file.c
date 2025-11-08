#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "sqlite3.h"

#define NR_OF_BATCHES 100000
#define TEXT_OFFSET 1
#define NUMERIC_OFFSET 9
#define INTEGER_OFFSET 14
#define REAL_OFFSET 21
#define BLOB_OFFSET 29
#define BLOB_END 48
#define INSERT_SQL_BEGINNING_LEN 29
#define BATCH_SIZE 1000

static const char* CREATE_TABLE_SQL = "CREATE TABLE test_data (x1 TEXT, x2 NUMERIC, x3 INTEGER, x4 REAL, x5);";
static const char* INSERT_SQL_BEGINNING = "INSERT INTO test_data VALUES ";

static void fill_insert_values(char* buf) {
    buf[0] = '\'';
    for (int i = TEXT_OFFSET; i < NUMERIC_OFFSET - 2; ++i) {
        buf[i] = (char)((rand() % 26) + 65);
    }
    buf[NUMERIC_OFFSET - 2] = '\'';
    buf[NUMERIC_OFFSET - 1] = ',';
    for (int i = NUMERIC_OFFSET; i < INTEGER_OFFSET - 1; ++i) {
        buf[i] = (char)((rand() % 10) + 48);
    }
    buf[INTEGER_OFFSET - 1] = ',';
    for (int i = INTEGER_OFFSET; i < REAL_OFFSET - 1; ++i) {
        if (i - INTEGER_OFFSET == 3) buf[i] = '.';
        else buf[i] = (char)((rand() % 10) + 48);
    }
    buf[REAL_OFFSET - 1] = ',';
    for (int i = REAL_OFFSET; i < BLOB_OFFSET - 2; ++i) {
        if (i - REAL_OFFSET == 3) buf[i] = '.';
        else buf[i] = (char)((rand() % 10) + 48);
    }
    buf[BLOB_OFFSET - 2] = ',';
    buf[BLOB_OFFSET - 1] = '\'';
    for (int i = BLOB_OFFSET; i < BLOB_END - 3; ++i) {
        buf[i] = (char)((rand() % 26) + 97);
    }
    buf[BLOB_END - 3] = '\'';
}

static int exec_insert_sql(sqlite3 *db, char* buffer) {
    sqlite3_stmt *insert_stmt;
    int total_query_size = INSERT_SQL_BEGINNING_LEN + BATCH_SIZE * (BLOB_END + 1) + 1;

    for (int i = 0; i < BATCH_SIZE; ++i) {
        buffer[INSERT_SQL_BEGINNING_LEN + i * (BLOB_END + 1)] = '(';
        fill_insert_values(&buffer[INSERT_SQL_BEGINNING_LEN + i * (BLOB_END + 1) + 1]);
        buffer[INSERT_SQL_BEGINNING_LEN + i * (BLOB_END + 1) + BLOB_END - 1] = ')';
        buffer[INSERT_SQL_BEGINNING_LEN + i * (BLOB_END + 1) + BLOB_END] = ',';
    }
    buffer[total_query_size - 2] = ';';
    buffer[total_query_size - 1] = '\0';

    int rc = sqlite3_prepare_v2(db, buffer, strlen(buffer), &insert_stmt, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Preparing insert stmt failed\n");
        return 1;
    }

    rc = sqlite3_step(insert_stmt);
    if(rc != SQLITE_DONE){
        fprintf(stderr, "Inserting into table failed\n");
        return 1;
    }
    sqlite3_finalize(insert_stmt);
    return 0;
}

int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *create_table_stmt;
    srand(time(NULL));
    
    if(argc != 2){
        fprintf(stderr, "Usage: %s DATABASE_FILE\n", argv[0]);
        return 1;
    }
    rc = sqlite3_open(argv[1], &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    rc = sqlite3_prepare_v2(db, CREATE_TABLE_SQL, strlen(CREATE_TABLE_SQL), &create_table_stmt, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Preparing create stmt failed\n");
        return 1;
    }

    rc = sqlite3_step(create_table_stmt);
    if(rc != SQLITE_DONE){
        fprintf(stderr, "Creating table failed\n");
        return 1;
    }
    sqlite3_finalize(create_table_stmt);

    char *buffer = malloc(INSERT_SQL_BEGINNING_LEN + BATCH_SIZE * (BLOB_END + 1) + 1);
    memcpy(buffer, INSERT_SQL_BEGINNING, INSERT_SQL_BEGINNING_LEN);

    for (int i = 0; i < NR_OF_BATCHES; ++i) {
        printf("%d\n", i);
        if (exec_insert_sql(db, buffer)) {
            free(buffer);
            return 1;
        }
    }

    free(buffer);
    sqlite3_close(db);
    return 0;
}
