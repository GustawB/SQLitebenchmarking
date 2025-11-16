#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite3.h"

static const char* SELECT_QUERY = "SELECT * FROM test_data";
static const char* SELECT_KEY_VALUE_TEMPLATE = "SELECT x1, x2, x3, x4, x5 FROM test_data WHERE x2 = ____";
static const char* JOIN_QUERY = "SELECT A.x1, B.x1 FROM test_data A LEFT JOIN test_data B on A.x1 = B.x1";

void random_select(char* query_buf) {
    for (int i = 0; i < 4; ++i) {
        int value = rand() % 10;
        if (i == 0) { while (value == 0) value = rand() % 10; }
        query_buf[i + strlen(SELECT_KEY_VALUE_TEMPLATE) - 4] = (char)(value + (int)'0');
    }
}

int main(int argc, char **argv) {
    sqlite3 *db;
    char* errMsg = NULL;
    int rc;
    struct timeval tval_before, tval_after, tval_result;
    srand(time(NULL));

    char* key_value_buf = malloc(strlen(SELECT_KEY_VALUE_TEMPLATE) + 1);
    strcpy(key_value_buf, SELECT_KEY_VALUE_TEMPLATE);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s DATABASE\n", argv[0]);
        return(1);
    }

    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    gettimeofday(&tval_before, NULL);
    for (int i = 0; i < 1; ++i) {
        random_select(key_value_buf);
        rc = sqlite3_exec(db, key_value_buf, NULL, 0, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
        }
    }
    gettimeofday(&tval_after, NULL);
    free(key_value_buf);

    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    sqlite3_close(db);
    return 0;
}