#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "sqlite3.h"

#define NR_OF_ROWS 100000
#define x1_pos 30
#define x1_len 6
#define x2_pos 39
#define x2_len 4
#define x3_pos 45
#define x3_len 6
#define x4_pos 53
#define x4_len 6
#define x5_pos 62
#define x5_len 16

const char* CREATE_TABLE_SQL = "CREATE TABLE test_data (x1 TEXT x2 NUMERIC x3 INTEGER x4 REAL x5);";
const char* INSERT_SQL = "INSERT INTO test_data VALUES(\"______\", ____, ______, ______, \"________________\");";

int exec_insert_sql(sqlite3 *db, char *zErrMsg) {
    char *curr_insert = malloc(strlen(INSERT_SQL));
    memcpy(curr_insert, INSERT_SQL, strlen(INSERT_SQL));
    char text[x1_len];
    char blob[x5_len];
    char integer[x2_len];
    char numeric[x3_len];
    srand(time(NULL));
    sqlite3_stmt *insert_stmt;

    for (int i = 0; i < x1_len; ++i) {
        text[i] = (char)((rand() % 26) + 65);
    }
    for (int i = 0; i < x2_len; ++i) {
        integer[i] = (char)((rand() % 10) + 48);
    }
    for (int i = 0; i < x3_len; ++i) {
        if (i == 3) numeric[i] = '.';
        else numeric[i] = (char)((rand() % 10) + 48);
    }
    for (int i = 0; i < x5_len; ++i) {
        blob[i] = (char)((rand() % 26) + 97);
    }

    memcpy(curr_insert + x1_pos, &text, x1_len);
    memcpy(curr_insert + x2_pos, &integer, x2_len);
    memcpy(curr_insert + x3_pos, &numeric, x3_len);
    memcpy(curr_insert + x4_pos, &numeric, x4_len);
    memcpy(curr_insert + x5_pos, &blob, x5_len);

    int rc = sqlite3_prepare_v2(db, curr_insert, strlen(curr_insert), &insert_stmt, &zErrMsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Preparing insert stmt failed: %s\n", zErrMsg);
        return 1;
    }

    rc = sqlite3_step(curr_insert);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Inserting into table failed: %s\n", zErrMsg);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *create_table_stmt;
    
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

    rc = sqlite3_prepare_v2(db, CREATE_TABLE_SQL, strlen(CREATE_TABLE_SQL), &create_table_stmt, &zErrMsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Preparing create stmt failed: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    rc = sqlite3_step(create_table_stmt);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Creating table failed: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    for (int i = 0; i < NR_OF_ROWS; ++i) {
        if (exec_insert_sql(db, zErrMsg)) {
            sqlite3_free(zErrMsg);
            return 1;
        }
    }

    sqlite3_close(db);
    return 0;
}