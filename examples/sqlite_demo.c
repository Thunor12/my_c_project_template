// SQLite amalgamation demo — build via: ./nob examples
#include <stdio.h>
#include <sqlite3.h>

int main(void)
{
    sqlite3 *db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK)
    {
        fprintf(stderr, "sqlite open failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT 1 + 1", -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        fprintf(stderr, "step failed\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    int value = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (value != 2)
    {
        fprintf(stderr, "unexpected result: %d\n", value);
        return 1;
    }

    printf("sqlite demo OK (SELECT 1 + 1 = %d)\n", value);
    return 0;
}
