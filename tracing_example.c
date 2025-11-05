#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
  
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");   
    return 0;
}

static int trace_callback(unsigned trace_flags, void *ctx, void *p, void *x) {
    const char *sql = (const char *)x;
    if(sql) {
        printf("TRACE: %s\n", sql);
    }
    return 0;
}
  
int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *stmt;
    
    if( argc!=3 ){
        fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
        return(1);
    }
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    rc = sqlite3_prepare_v2(db, argv[2], strlen(argv[2]), &stmt, NULL);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "Preparing failed: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_trace_v2(db, SQLITE_TRACE_STMT, trace_callback, NULL);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "Tracing failed: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    return 0;
}